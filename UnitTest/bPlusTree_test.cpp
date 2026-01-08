#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include <set>
#include <chrono>
#include "bPlusTree.h"

// Include your B+ tree implementation here
// #include "bplustree.h"


// Fixture class for B+ Tree tests
template<int ORDER = 128>
class BPlusTreeTest : public ::testing::Test {
protected:
    std::unique_ptr<BPlusTree<int, ORDER>> tree;
    std::vector<Data*> test_data;
    
    void SetUp() override {
        tree = std::make_unique<BPlusTree<int, ORDER>>();
    }
    
    void TearDown() override {
        // Clean up all data after the test is done
        for (auto d : test_data) {
            delete d;
        }
        test_data.clear();
    }
    
};

// Instantiate test suite with different orders
using BPlusTreeTestSmall = BPlusTreeTest<5>;
using BPlusTreeTestMedium = BPlusTreeTest<32>;
using BPlusTreeTestLarge = BPlusTreeTest<128>;

// Basic Insert and Search Tests
TEST_F(BPlusTreeTestSmall, InsertSingleElement) {
    Data* d = new Data(1, "First Record");
    test_data.push_back(d);
    tree->insert(1, d);
    
    EXPECT_EQ(tree->size(), 1);
    Data* result = tree->search(1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getRecordID(), 1);
    EXPECT_EQ(result->getData(), "First Record");
}

TEST_F(BPlusTreeTestSmall, InsertMultipleElements) {
    for (int i = 0; i < 10; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    EXPECT_EQ(tree->size(), 10);
    
    for (int i = 0; i < 10; ++i) {
        Data* result = tree->search(i);
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->getRecordID(), i);
    }
}

TEST_F(BPlusTreeTestSmall, SearchNonExistentKey) {
    Data* d = new Data(5, "Record 5");
    test_data.push_back(d);
    tree->insert(5, d);
    
    Data* result = tree->search(10);
    EXPECT_EQ(result, nullptr);
}

TEST_F(BPlusTreeTestSmall, InsertAscendingOrder) {
    const int n = 100;
    for (int i = 0; i < n; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    EXPECT_EQ(tree->size(), n);
    
    for (int i = 0; i < n; ++i) {
        Data* result = tree->search(i);
        ASSERT_NE(result, nullptr) << "Failed to find key: " << i;
        EXPECT_EQ(result->getRecordID(), i);
    }
}

TEST_F(BPlusTreeTestSmall, InsertDescendingOrder) {
    const int n = 100;
    for (int i = n - 1; i >= 0; --i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    EXPECT_EQ(tree->size(), n);
    
    for (int i = 0; i < n; ++i) {
        Data* result = tree->search(i);
        ASSERT_NE(result, nullptr) << "Failed to find key: " << i;
        EXPECT_EQ(result->getRecordID(), i);
    }
}

TEST_F(BPlusTreeTestSmall, InsertRandomOrder) {
    const int n = 100;
    std::vector<int> keys(n);
    for (int i = 0; i < n; ++i) keys[i] = i;
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(), g);
    
    for (int key : keys) {
        Data* d = new Data(key, "Record " + std::to_string(key));
        test_data.push_back(d);
        tree->insert(key, d);
    }
    
    EXPECT_EQ(tree->size(), n);
    
    for (int i = 0; i < n; ++i) {
        Data* result = tree->search(i);
        ASSERT_NE(result, nullptr) << "Failed to find key: " << i;
        EXPECT_EQ(result->getRecordID(), i);
    }
}

// Range Query Tests
TEST_F(BPlusTreeTestSmall, RangeQueryEmpty) {
    auto results = tree->rangeQuery(0, 10);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(BPlusTreeTestSmall, RangeQuerySingleElement) {
    Data* d = new Data(5, "Record 5");
    test_data.push_back(d);
    tree->insert(5, d);
    
    auto results = tree->rangeQuery(5, 5);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->getRecordID(), 5);
}

TEST_F(BPlusTreeTestSmall, RangeQueryMultipleElements) {
    for (int i = 0; i < 20; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    auto results = tree->rangeQuery(5, 10);
    EXPECT_EQ(results.size(), 6); // 5, 6, 7, 8, 9, 10
    
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_EQ(results[i]->getRecordID(), 5 + i);
    }
}

TEST_F(BPlusTreeTestSmall, RangeQueryOutOfBounds) {
    for (int i = 0; i < 10; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    auto results = tree->rangeQuery(20, 30);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(BPlusTreeTestSmall, RangeQueryPartialOverlap) {
    for (int i = 0; i < 10; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    auto results = tree->rangeQuery(5, 15);
    EXPECT_EQ(results.size(), 5); // 5, 6, 7, 8, 9
}

TEST_F(BPlusTreeTestSmall, RangeQueryFullRange) {
    const int n = 20;
    for (int i = 0; i < n; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    auto results = tree->rangeQuery(0, n - 1);
    EXPECT_EQ(results.size(), n);
}

// Stress Tests
TEST_F(BPlusTreeTestMedium, StressTestLargeDataset) {
    const int n = 10000;
    for (int i = 0; i < n; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    EXPECT_EQ(tree->size(), n);
    
    // Verify all elements
    for (int i = 0; i < n; ++i) {
        Data* result = tree->search(i);
        ASSERT_NE(result, nullptr) << "Failed at key: " << i;
        EXPECT_EQ(result->getRecordID(), i);
    }
}

TEST_F(BPlusTreeTestMedium, StressTestRandomInsertAndSearch) {
    const int n = 5000;
    std::vector<int> keys(n);
    for (int i = 0; i < n; ++i) keys[i] = i;
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(), g);
    
    for (int key : keys) {
        Data* d = new Data(key, "Record " + std::to_string(key));
        test_data.push_back(d);
        tree->insert(key, d);
    }
    
    std::shuffle(keys.begin(), keys.end(), g);
    
    for (int key : keys) {
        Data* result = tree->search(key);
        ASSERT_NE(result, nullptr) << "Failed to find key: " << key;
        EXPECT_EQ(result->getRecordID(), key);
    }
}

TEST_F(BPlusTreeTestLarge, PerformanceBenchmarkInsert) {
    const int n = 100000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < n; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(tree->size(), n);
    std::cout << "Inserted " << n << " elements in " << duration.count() << " ms" << std::endl;
}

TEST_F(BPlusTreeTestLarge, PerformanceBenchmarkSearch) {
    const int n = 100000;
    
    for (int i = 0; i < n; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis(0, n - 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    const int num_searches = 10000;
    for (int i = 0; i < num_searches; ++i) {
        int key = dis(g);
        Data* result = tree->search(key);
        ASSERT_NE(result, nullptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Performed " << num_searches << " random searches in " 
              << duration.count() << " ms" << std::endl;
}

// Edge Cases
TEST_F(BPlusTreeTestSmall, InsertDuplicateKeys) {
    Data* d1 = new Data(1, "First");
    Data* d2 = new Data(1, "Second");
    test_data.push_back(d1);
    test_data.push_back(d2);
    
    tree->insert(1, d1);
    tree->insert(1, d2);
    
    // Note: This behavior depends on your implementation
    // You may want to handle duplicates differently
    EXPECT_EQ(tree->size(), 2);
}

TEST_F(BPlusTreeTestSmall, EmptyTreeSearch) {
    Data* result = tree->search(0);
    EXPECT_EQ(result, nullptr);
}

TEST_F(BPlusTreeTestSmall, EmptyTreeRangeQuery) {
    auto results = tree->rangeQuery(0, 100);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(BPlusTreeTestSmall, NegativeKeys) {
    for (int i = -10; i < 10; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    for (int i = -10; i < 10; ++i) {
        Data* result = tree->search(i);
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->getRecordID(), i);
    }
}

TEST_F(BPlusTreeTestSmall, RangeQueryWithNegativeKeys) {
    for (int i = -10; i < 10; ++i) {
        Data* d = new Data(i, "Record " + std::to_string(i));
        test_data.push_back(d);
        tree->insert(i, d);
    }
    
    auto results = tree->rangeQuery(-5, 5);
    EXPECT_EQ(results.size(), 11); // -5 to 5 inclusive
}

// Main function
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}