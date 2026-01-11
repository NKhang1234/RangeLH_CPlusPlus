#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <random>
#include <memory>
#include "bPlusTree.h"

// Test Fixture
class BPlusTreeTest : public ::testing::Test {
protected:
    std::vector<Data*> test_data;
    
    void SetUp() override {
        test_data.clear();
    }
    
    void TearDown() override {
        for (Data* d : test_data) {
            delete d;
        }
        test_data.clear();
    }
    
    Data* createData(int id, const std::string& content) {
        Data* d = new Data(id, content);
        test_data.push_back(d);
        return d;
    }
};

// ============================================================================
// BASIC FUNCTIONALITY TESTS
// ============================================================================

TEST_F(BPlusTreeTest, ConstructorDefaultOrder) {
    BPlusTree tree;
    EXPECT_EQ(tree.getOrder(), 3);
    EXPECT_TRUE(tree.isEmpty());
}

TEST_F(BPlusTreeTest, ConstructorCustomOrder) {
    BPlusTree tree(5);
    EXPECT_EQ(tree.getOrder(), 5);
    EXPECT_TRUE(tree.isEmpty());
}

TEST_F(BPlusTreeTest, ConstructorMinimumOrderEnforcement) {
    BPlusTree tree(2);
    EXPECT_EQ(tree.getOrder(), 3);
    
    BPlusTree tree2(1);
    EXPECT_EQ(tree2.getOrder(), 3);
}

TEST_F(BPlusTreeTest, EmptyTreeSearch) {
    BPlusTree tree(3);
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(tree.search(10), nullptr);
    EXPECT_EQ(tree.search(0), nullptr);
    EXPECT_EQ(tree.search(-5), nullptr);
}

TEST_F(BPlusTreeTest, SingleInsertion) {
    BPlusTree tree(3);
    Data* d = createData(10, "Record 10");
    
    tree.insert(d);
    EXPECT_FALSE(tree.isEmpty());
    
    Data* found = tree.search(10);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getRecordID(), 10);
    EXPECT_EQ(found->getData(), "Record 10");
}

TEST_F(BPlusTreeTest, SingleInsertionRootIsLeaf) {
    BPlusTree tree(3);
    Data* d = createData(10, "Record 10");
    
    tree.insert(d);
    
    Node* root = tree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(root->isLeaf());
}

// ============================================================================
// MULTIPLE INSERTIONS TESTS
// ============================================================================

TEST_F(BPlusTreeTest, MultipleInsertionsAscending) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 5; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    for (int i = 1; i <= 5; i++) {
        Data* found = tree.search(i * 10);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->getRecordID(), i * 10);
        EXPECT_EQ(found->getData(), "Record " + std::to_string(i * 10));
    }
}

TEST_F(BPlusTreeTest, MultipleInsertionsDescending) {
    BPlusTree tree(3);
    
    for (int i = 5; i >= 1; i--) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    for (int i = 1; i <= 5; i++) {
        Data* found = tree.search(i * 10);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->getRecordID(), i * 10);
    }
}

TEST_F(BPlusTreeTest, MultipleInsertionsRandom) {
    BPlusTree tree(3);
    
    std::vector<int> keys = {15, 10, 25, 5, 20, 30, 35};
    for (int key : keys) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    for (int key : keys) {
        Data* found = tree.search(key);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->getRecordID(), key);
    }
}

// ============================================================================
// ORDERING AND TRAVERSAL TESTS
// ============================================================================

TEST_F(BPlusTreeTest, OrderMaintainedAfterInsertions) {
    BPlusTree tree(3);
    
    std::vector<int> keys = {15, 10, 25, 5, 20, 30, 35};
    for (int key : keys) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    std::vector<int> ordered_keys = tree.getAllKeysInOrder();
    std::vector<int> expected = {5, 10, 15, 20, 25, 30, 35};
    
    EXPECT_EQ(ordered_keys, expected);
}

TEST_F(BPlusTreeTest, DataPointersInCorrectOrder) {
    BPlusTree tree(3);
    
    std::vector<int> keys = {30, 10, 20};
    for (int key : keys) {
        tree.insert(createData(key, "Data_" + std::to_string(key)));
    }
    
    std::vector<Data*> data_list = tree.getAllDataInOrder();
    ASSERT_EQ(data_list.size(), 3);
    
    EXPECT_EQ(data_list[0]->getRecordID(), 10);
    EXPECT_EQ(data_list[1]->getRecordID(), 20);
    EXPECT_EQ(data_list[2]->getRecordID(), 30);
}

TEST_F(BPlusTreeTest, LeafLinkedListTraversal) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 10; i++) {
        tree.insert(createData(i, "Record " + std::to_string(i)));
    }
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), 10);
    
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(keys[i], i + 1);
    }
}

// ============================================================================
// SEARCH TESTS
// ============================================================================

TEST_F(BPlusTreeTest, SearchNonExistentKeyEmptyTree) {
    BPlusTree tree(3);
    EXPECT_EQ(tree.search(100), nullptr);
}

TEST_F(BPlusTreeTest, SearchNonExistentKeyInPopulatedTree) {
    BPlusTree tree(3);
    
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    tree.insert(createData(30, "Record 30"));
    
    EXPECT_EQ(tree.search(15), nullptr);
    EXPECT_EQ(tree.search(5), nullptr);
    EXPECT_EQ(tree.search(100), nullptr);
    EXPECT_EQ(tree.search(25), nullptr);
}

TEST_F(BPlusTreeTest, SearchBoundaryKeys) {
    BPlusTree tree(3);
    
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    tree.insert(createData(30, "Record 30"));
    
    EXPECT_NE(tree.search(10), nullptr); // Smallest
    EXPECT_NE(tree.search(30), nullptr); // Largest
}

// ============================================================================
// SPLIT OPERATION TESTS
// ============================================================================

TEST_F(BPlusTreeTest, LeafSplitOrder3) {
    BPlusTree tree(3);
    
    // Order 3: max 2 keys before split
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    
    Node* root = tree.getRoot();
    EXPECT_TRUE(root->isLeaf());
    
    tree.insert(createData(30, "Record 30")); // Triggers split
    
    // After split, root should be internal node
    root = tree.getRoot();
    EXPECT_FALSE(root->isLeaf());
    
    // All keys should still be searchable
    EXPECT_NE(tree.search(10), nullptr);
    EXPECT_NE(tree.search(20), nullptr);
    EXPECT_NE(tree.search(30), nullptr);
}

TEST_F(BPlusTreeTest, MultipleSplits) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 10; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    // Verify all keys are searchable
    for (int i = 1; i <= 10; i++) {
        Data* found = tree.search(i * 10);
        ASSERT_NE(found, nullptr) << "Key " << (i * 10) << " not found";
        EXPECT_EQ(found->getRecordID(), i * 10);
    }
    
    // Verify order is maintained
    std::vector<int> keys = tree.getAllKeysInOrder();
    for (size_t i = 1; i < keys.size(); i++) {
        EXPECT_LT(keys[i-1], keys[i]) << "Keys not in order at index " << i;
    }
}

TEST_F(BPlusTreeTest, InternalNodeSplit) {
    BPlusTree tree(3);
    
    // Insert enough elements to trigger internal node split
    for (int i = 1; i <= 20; i++) {
        tree.insert(createData(i, "Record " + std::to_string(i)));
    }
    
    // Verify all are searchable
    for (int i = 1; i <= 20; i++) {
        EXPECT_NE(tree.search(i), nullptr) << "Key " << i << " not found";
    }
    
    // Verify ordering
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), 20);
    for (int i = 0; i < 20; i++) {
        EXPECT_EQ(keys[i], i + 1);
    }
}

// ============================================================================
// LARGE DATASET TESTS
// ============================================================================

TEST_F(BPlusTreeTest, LargeDatasetSequential) {
    BPlusTree tree(4);
    const int num_records = 100;
    
    for (int i = 0; i < num_records; i++) {
        tree.insert(createData(i, "Record " + std::to_string(i)));
    }
    
    // Verify all insertions
    for (int i = 0; i < num_records; i++) {
        Data* found = tree.search(i);
        ASSERT_NE(found, nullptr) << "Failed to find key " << i;
        EXPECT_EQ(found->getRecordID(), i);
    }
    
    // Verify order
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), num_records);
    for (int i = 0; i < num_records; i++) {
        EXPECT_EQ(keys[i], i);
    }
}

TEST_F(BPlusTreeTest, LargeDatasetRandom) {
    BPlusTree tree(5);
    const int num_records = 100;
    
    std::vector<int> keys;
    for (int i = 0; i < num_records; i++) {
        keys.push_back(i);
    }
    
    std::random_device rd;
    std::mt19937 g(42); // Fixed seed for reproducibility
    std::shuffle(keys.begin(), keys.end(), g);
    
    for (int key : keys) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    // Verify all insertions
    for (int i = 0; i < num_records; i++) {
        Data* found = tree.search(i);
        ASSERT_NE(found, nullptr) << "Failed to find key " << i;
    }
    
    // Verify sorted order
    std::vector<int> ordered = tree.getAllKeysInOrder();
    EXPECT_EQ(ordered.size(), num_records);
    for (int i = 0; i < num_records; i++) {
        EXPECT_EQ(ordered[i], i);
    }
}

TEST_F(BPlusTreeTest, StressTestVariousOrders) {
    for (int order = 3; order <= 10; order++) {
        BPlusTree tree(order);
        
        for (int i = 0; i < 50; i++) {
            tree.insert(createData(i, "Record " + std::to_string(i)));
        }
        
        for (int i = 0; i < 50; i++) {
            ASSERT_NE(tree.search(i), nullptr) 
                << "Order " << order << " failed to find key " << i;
        }
        
        std::vector<int> keys = tree.getAllKeysInOrder();
        EXPECT_EQ(keys.size(), 50) << "Order " << order << " has wrong number of keys";
    }
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(BPlusTreeTest, NegativeKeys) {
    BPlusTree tree(3);
    
    tree.insert(createData(-10, "Record -10"));
    tree.insert(createData(-5, "Record -5"));
    tree.insert(createData(0, "Record 0"));
    tree.insert(createData(5, "Record 5"));
    
    EXPECT_NE(tree.search(-10), nullptr);
    EXPECT_NE(tree.search(-5), nullptr);
    EXPECT_NE(tree.search(0), nullptr);
    EXPECT_NE(tree.search(5), nullptr);
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    std::vector<int> expected = {-10, -5, 0, 5};
    EXPECT_EQ(keys, expected);
}

TEST_F(BPlusTreeTest, MixedPositiveNegativeKeys) {
    BPlusTree tree(4);
    
    std::vector<int> input = {50, -30, 20, -10, 0, 40, -20, 10};
    for (int key : input) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    std::vector<int> expected = {-30, -20, -10, 0, 10, 20, 40, 50};
    EXPECT_EQ(keys, expected);
}

TEST_F(BPlusTreeTest, DuplicateKeysHandling) {
    BPlusTree tree(3);
    
    Data* d1 = createData(10, "First Record 10");
    Data* d2 = createData(10, "Second Record 10");
    Data* d3 = createData(20, "Record 20");
    
    tree.insert(d1);
    tree.insert(d2);
    tree.insert(d3);
    
    // Search should find one of the duplicate entries
    Data* found = tree.search(10);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getRecordID(), 10);
}

TEST_F(BPlusTreeTest, LargeKeyValues) {
    BPlusTree tree(3);
    
    tree.insert(createData(1000000, "Large 1"));
    tree.insert(createData(2000000, "Large 2"));
    tree.insert(createData(500000, "Large 3"));
    
    EXPECT_NE(tree.search(1000000), nullptr);
    EXPECT_NE(tree.search(2000000), nullptr);
    EXPECT_NE(tree.search(500000), nullptr);
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys[0], 500000);
    EXPECT_EQ(keys[1], 1000000);
    EXPECT_EQ(keys[2], 2000000);
}

TEST_F(BPlusTreeTest, SingleKeyRepeatedSearch) {
    BPlusTree tree(3);
    Data* d = createData(42, "Answer");
    tree.insert(d);
    
    for (int i = 0; i < 100; i++) {
        Data* found = tree.search(42);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->getData(), "Answer");
    }
}

// ============================================================================
// DATA INTEGRITY TESTS
// ============================================================================

TEST_F(BPlusTreeTest, DataIntegrityAfterMultipleSplits) {
    BPlusTree tree(3);
    
    std::vector<std::pair<int, std::string>> test_cases = {
        {5, "Alpha"}, {15, "Beta"}, {25, "Gamma"}, {35, "Delta"},
        {10, "Epsilon"}, {20, "Zeta"}, {30, "Eta"}, {40, "Theta"}
    };
    
    for (const auto& tc : test_cases) {
        tree.insert(createData(tc.first, tc.second));
    }
    
    for (const auto& tc : test_cases) {
        Data* found = tree.search(tc.first);
        ASSERT_NE(found, nullptr) << "Key " << tc.first << " not found";
        EXPECT_EQ(found->getData(), tc.second);
    }
}

TEST_F(BPlusTreeTest, PointerConsistencyInLeafNodes) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 20; i++) {
        Data* d = createData(i, "Data_" + std::to_string(i));
        tree.insert(d);
    }
    
    std::vector<Data*> data_list = tree.getAllDataInOrder();
    
    for (size_t i = 0; i < data_list.size(); i++) {
        EXPECT_EQ(data_list[i]->getRecordID(), static_cast<int>(i + 1));
        EXPECT_EQ(data_list[i]->getData(), "Data_" + std::to_string(i + 1));
    }
}

// ============================================================================
// BASIC DELETE TESTS
// ============================================================================

TEST_F(BPlusTreeTest, DeleteFromEmptyTree) {
    BPlusTree tree(3);
    EXPECT_FALSE(tree.remove(10));
    EXPECT_TRUE(tree.isEmpty());
}

TEST_F(BPlusTreeTest, DeleteNonExistentKey) {
    BPlusTree tree(3);
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    
    EXPECT_FALSE(tree.remove(15));
    EXPECT_FALSE(tree.remove(5));
    EXPECT_FALSE(tree.remove(100));
}

TEST_F(BPlusTreeTest, DeleteSingleElementTree) {
    BPlusTree tree(3);
    tree.insert(createData(10, "Record 10"));

    tree.printTree();    

    EXPECT_TRUE(tree.remove(10));
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(tree.search(10), nullptr);
}

TEST_F(BPlusTreeTest, DeleteAndVerifyRemoval) {
    BPlusTree tree(3);
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    tree.insert(createData(30, "Record 30"));
    
    EXPECT_TRUE(tree.remove(20));
    EXPECT_EQ(tree.search(20), nullptr);
    EXPECT_NE(tree.search(10), nullptr);
    EXPECT_NE(tree.search(30), nullptr);
}

TEST_F(BPlusTreeTest, DeleteFirstKey) {
    BPlusTree tree(3);
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    tree.insert(createData(30, "Record 30"));
    
    EXPECT_TRUE(tree.remove(10));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    std::vector<int> expected = {20, 30};
    EXPECT_EQ(keys, expected);
}

TEST_F(BPlusTreeTest, DeleteLastKey) {
    BPlusTree tree(3);
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    tree.insert(createData(30, "Record 30"));
    
    EXPECT_TRUE(tree.remove(30));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    std::vector<int> expected = {10, 20};
    EXPECT_EQ(keys, expected);
}

TEST_F(BPlusTreeTest, DeleteMiddleKey) {
    BPlusTree tree(3);
    tree.insert(createData(10, "Record 10"));
    tree.insert(createData(20, "Record 20"));
    tree.insert(createData(30, "Record 30"));
    
    EXPECT_TRUE(tree.remove(20));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    std::vector<int> expected = {10, 30};
    EXPECT_EQ(keys, expected);
}

// ============================================================================
// MULTIPLE DELETE TESTS
// ============================================================================

TEST_F(BPlusTreeTest, DeleteAllKeysSequentially) {
    BPlusTree tree(3);
    std::vector<int> keys_to_insert = {10, 20, 30, 40, 50};
    
    for (int key : keys_to_insert) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    for (int key : keys_to_insert) {
        EXPECT_TRUE(tree.remove(key));
        EXPECT_EQ(tree.search(key), nullptr);
    }
    
    EXPECT_TRUE(tree.isEmpty());
}

TEST_F(BPlusTreeTest, DeleteAllKeysReverseOrder) {
    BPlusTree tree(3);
    std::vector<int> keys_to_insert = {10, 20, 30, 40, 50};
    
    for (int key : keys_to_insert) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    for (auto it = keys_to_insert.rbegin(); it != keys_to_insert.rend(); ++it) {
        EXPECT_TRUE(tree.remove(*it));
        EXPECT_EQ(tree.search(*it), nullptr);
    }
    
    EXPECT_TRUE(tree.isEmpty());
}

TEST_F(BPlusTreeTest, DeleteAlternatingKeys) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 10; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    // Delete even-indexed keys
    for (int i = 2; i <= 10; i += 2) {
        EXPECT_TRUE(tree.remove(i * 10));
    }
    
    // Verify odd-indexed keys remain
    for (int i = 1; i <= 10; i++) {
        if (i % 2 == 0) {
            EXPECT_EQ(tree.search(i * 10), nullptr);
        } else {
            EXPECT_NE(tree.search(i * 10), nullptr);
        }
    }
}

// ============================================================================
// UNDERFLOW AND REBALANCING TESTS
// ============================================================================

TEST_F(BPlusTreeTest, LeafBorrowFromLeftSibling) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 5; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    // This should trigger borrowing from left sibling
    EXPECT_TRUE(tree.remove(40));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), 4);
    
    // All remaining keys should be searchable
    EXPECT_NE(tree.search(10), nullptr);
    EXPECT_NE(tree.search(20), nullptr);
    EXPECT_NE(tree.search(30), nullptr);
    EXPECT_NE(tree.search(50), nullptr);
}

TEST_F(BPlusTreeTest, LeafBorrowFromRightSibling) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 5; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    // This should trigger borrowing from right sibling
    EXPECT_TRUE(tree.remove(20));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), 4);
    
    EXPECT_NE(tree.search(10), nullptr);
    EXPECT_NE(tree.search(30), nullptr);
    EXPECT_NE(tree.search(40), nullptr);
    EXPECT_NE(tree.search(50), nullptr);
}

TEST_F(BPlusTreeTest, LeafMergeWithSibling) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 7; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    // Delete keys that will cause merging
    EXPECT_TRUE(tree.remove(30));
    EXPECT_TRUE(tree.remove(40));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), 5);
    
    // Verify ordering is maintained
    for (size_t i = 1; i < keys.size(); i++) {
        EXPECT_LT(keys[i-1], keys[i]);
    }
}

TEST_F(BPlusTreeTest, InternalNodeMerge) {
    BPlusTree tree(3);
    
    // Insert enough to create multi-level tree
    for (int i = 1; i <= 15; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    // Delete multiple keys to trigger internal node merge
    for (int i = 1; i <= 8; i++) {
        EXPECT_TRUE(tree.remove(i * 10));
    }
    
    // Verify remaining keys
    for (int i = 9; i <= 15; i++) {
        EXPECT_NE(tree.search(i * 10), nullptr) 
            << "Key " << (i * 10) << " should exist";
    }
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    EXPECT_EQ(keys.size(), 7);
}

TEST_F(BPlusTreeTest, RootReduction) {
    BPlusTree tree(3);
    
    // Create a tree with multiple levels
    for (int i = 1; i <= 10; i++) {
        tree.insert(createData(i * 10, "Record " + std::to_string(i * 10)));
    }
    
    Node* root_before = tree.getRoot();
    EXPECT_FALSE(root_before->isLeaf());
    
    // Delete keys to reduce tree height
    for (int i = 1; i <= 7; i++) {
        EXPECT_TRUE(tree.remove(i * 10));
    }
    
    // Remaining keys should still be accessible
    for (int i = 8; i <= 10; i++) {
        EXPECT_NE(tree.search(i * 10), nullptr);
    }
}

// ============================================================================
// ORDERING PRESERVATION TESTS
// ============================================================================

TEST_F(BPlusTreeTest, OrderPreservedAfterDeletes) {
    BPlusTree tree(3);
    
    std::vector<int> insert_order = {50, 20, 70, 10, 30, 60, 80, 5, 15, 25};
    for (int key : insert_order) {
        tree.insert(createData(key, "Record " + std::to_string(key)));
    }
    
    // Delete some keys
    EXPECT_TRUE(tree.remove(20));
    EXPECT_TRUE(tree.remove(60));
    EXPECT_TRUE(tree.remove(5));
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    
    // Verify remaining keys are in sorted order
    for (size_t i = 1; i < keys.size(); i++) {
        EXPECT_LT(keys[i-1], keys[i]) << "Keys not in order at index " << i;
    }
}

TEST_F(BPlusTreeTest, LeafLinkedListIntegrityAfterDelete) {
    BPlusTree tree(3);
    
    for (int i = 1; i <= 20; i++) {
        tree.insert(createData(i, "Record " + std::to_string(i)));
    }
    
    // Delete multiple keys
    for (int i = 5; i <= 15; i += 2) {
        EXPECT_TRUE(tree.remove(i));
    }
    
    std::vector<int> keys = tree.getAllKeysInOrder();
    
    // Verify all keys are in ascending order
    for (size_t i = 1; i < keys.size(); i++) {
        EXPECT_LT(keys[i-1], keys[i]);
    }
    
    // Verify correct number of keys
    EXPECT_EQ(keys.size(), 14);
}