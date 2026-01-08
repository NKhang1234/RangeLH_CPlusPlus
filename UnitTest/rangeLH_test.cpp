#include <gtest/gtest.h>
#include "rangeLH.h"

// ------------------------------------------------------------
// Test Fixture
// ------------------------------------------------------------
class RangeLHTest : public ::testing::Test {
protected:
    void SetUp() override {
        index = std::make_unique<RangeLH>(
            /* master_init_num_bucket */ 4,
            /* master_split_policy */ 0.75,
            /* expected_n_items */ 100,
            /* fp_prob_bloom_RF */ 0.01,
            /* delta_bloom_RF */ 2,
            /* key_length */ 64,
            /* max_bytes_string */ 8,
            /* float_scale */ 1000
        );
    }
    void TearDown() override {
        // Clean up all data after the test is done
        for (auto d : test_data) {
            delete d;
        }
        test_data.clear();
    }

    std::unique_ptr<RangeLH> index;
    std::vector<Data*> test_data; // Track all 'new' data here
};

// ------------------------------------------------------------
// Insert + Point Lookup (uint64)
// ------------------------------------------------------------
TEST_F(RangeLHTest, InsertAndPointLookupUint64) {
    Data* d1 = new Data{100, "10"};
    test_data.push_back(d1);

    EXPECT_TRUE(index->insert(static_cast<uint64_t>(100ULL), *d1));

    auto res = index->point_lookup(static_cast<uint64_t>(100ULL));
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value()->getData(), d1->getData());

}

// ------------------------------------------------------------
// Insert + Update Existing Key
// ------------------------------------------------------------
TEST_F(RangeLHTest, InsertUpdatesExistingKey) {
    Data* d1 = new Data{10, "10`"};
    Data* d2 = new Data{20, "20"};
    test_data.push_back(d1);
    test_data.push_back(d2);

    EXPECT_TRUE(index->insert(static_cast<uint64_t>(42ULL), *d1));
    EXPECT_TRUE(index->insert(static_cast<uint64_t>(42ULL), *d2)); // update

    auto res = index->point_lookup(static_cast<uint64_t>(42ULL));
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value()->getData(), d2->getData());
}

// ------------------------------------------------------------
// Point Lookup Missing Key
// ------------------------------------------------------------
TEST_F(RangeLHTest, PointLookupNotFound) {
    auto res = index->point_lookup(static_cast<uint64_t>(999ULL));
    EXPECT_FALSE(res.has_value());
}

// ------------------------------------------------------------
// Range Lookup Basic
// ------------------------------------------------------------
TEST_F(RangeLHTest, RangeLookupBasic) {
    Data* d1 = new Data{1, "1"};
    Data* d2 = new Data{2, "2"};
    Data* d3 = new Data{3, "3"};
    test_data.push_back(d1);
    test_data.push_back(d2);
    test_data.push_back(d3);

    index->insert(static_cast<uint64_t>(10ULL), *d1);
    index->insert(static_cast<uint64_t>(20ULL), *d2);
    index->insert(static_cast<uint64_t>(30ULL), *d3);

    // index->print_worker_chain();

    auto res = index->range_lookup(static_cast<uint64_t>(10ULL), static_cast<uint64_t>(30ULL));
    ASSERT_TRUE(res.has_value());
    ASSERT_EQ(res->size(), 3);

    EXPECT_EQ((*res)[0]->getData(), d1->getData());
    EXPECT_EQ((*res)[1]->getData(), d2->getData());
    EXPECT_EQ((*res)[2]->getData(), d3->getData());
}

// ------------------------------------------------------------
// Range Lookup Partial
// ------------------------------------------------------------
TEST_F(RangeLHTest, RangeLookupPartial) {
    Data* d1 = new Data{1, "1"};
    Data* d2 = new Data{2, "2"};
    Data* d3 = new Data{3, "3"};
    test_data.push_back(d1);
    test_data.push_back(d2);
    test_data.push_back(d3);

    index->insert(static_cast<uint64_t>(10ULL), *d1);
    index->insert(static_cast<uint64_t>(20ULL), *d2);
    index->insert(static_cast<uint64_t>(30ULL), *d3);
    auto res = index->range_lookup(static_cast<uint64_t>(15ULL), static_cast<uint64_t>(25ULL));
    ASSERT_TRUE(res.has_value());
    ASSERT_EQ(res->size(), 1);
    EXPECT_EQ((*res)[0]->getData(), d2->getData());
}

// ------------------------------------------------------------
// Range Lookup Empty Result
// ------------------------------------------------------------
TEST_F(RangeLHTest, RangeLookupEmpty) {
    Data* d1 = new Data{1, "1"};
    test_data.push_back(d1);

    index->insert(static_cast<uint64_t>(10ULL), *d1);

    auto res = index->range_lookup(static_cast<uint64_t>(20ULL), static_cast<uint64_t>(30ULL));
    EXPECT_FALSE(res.has_value());

}

// ------------------------------------------------------------
// String Key Insert + Lookup
// ------------------------------------------------------------
TEST_F(RangeLHTest, StringKeyInsertLookup) {
    Data* d1 = new Data{100, "100"};
    test_data.push_back(d1);

    EXPECT_TRUE(index->insert(std::string("apple"), *d1));

    auto res = index->point_lookup(std::string("apple"));
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value()->getData(), d1->getData());
}

// // ------------------------------------------------------------
// // String Range Lookup (Lexicographic)
// // ------------------------------------------------------------
// TEST_F(RangeLHTest, StringRangeLookup) {
//     Data* d1 = new Data{1, "1"};
//     Data* d2 = new Data{2, "2"};
//     Data* d3 = new Data{3, "3"};

//     index->insert("apple", *d1);
//     index->insert("banana", *d2);
//     index->insert("carrot", *d3);
//     // index->print_worker_chain();

//     auto res = index->range_lookup("apple", "banana");
//     ASSERT_TRUE(res.has_value());
//     ASSERT_EQ(res->size(), 2);

//     EXPECT_EQ((*res)[0]->getData(), d1->getData());
//     EXPECT_EQ((*res)[1]->getData(), d2->getData());

//     delete d1;
//     delete d2;
//     delete d3;
// }

// // ------------------------------------------------------------
// // Pattern Lookup (prefix%)
// // ------------------------------------------------------------
// TEST_F(RangeLHTest, PatternLookupPrefix) {
//     Data* d1 = new Data{1, "a1"};
//     Data* d2 = new Data{2, "a2"};
//     Data* d3 = new Data{3, "a3"};

//     index->insert("user1", *d1);
//     index->insert("user2", *d2);
//     index->insert("admin", *d3);
//     // index->print_worker_chain();

//     auto res = index->pattern_lookup("user%");
//     ASSERT_TRUE(res.has_value());
//     ASSERT_EQ(res->size(), 2);

//     EXPECT_EQ((*res)[0]->getData(), d1->getData());
//     EXPECT_EQ((*res)[1]->getData(), d2->getData());
    
//     delete d1;
//     delete d2;
// }

// ------------------------------------------------------------
// Float Key Insert + Range
// ------------------------------------------------------------
TEST_F(RangeLHTest, FloatKeyInsertRange) {
    Data* d1 = new Data{1, "1"};
    Data* d2 = new Data{2, "2"};
    test_data.push_back(d1);
    test_data.push_back(d2);

    index->insert(1.5, *d1);
    index->insert(2.5, *d2);

    auto res = index->range_lookup(1.0, 2.0);
    ASSERT_TRUE(res.has_value());
    ASSERT_EQ(res->size(), 1);
    EXPECT_EQ((*res)[0]->getData(), d1->getData());

}

// ------------------------------------------------------------
// Remove Existing Key
// ------------------------------------------------------------
TEST_F(RangeLHTest, RemoveKey) {
    Data* d1 = new Data{10, "10"};
    test_data.push_back(d1);

    index->insert(static_cast<uint64_t>(50ULL), *d1);
    EXPECT_TRUE(index->remove(static_cast<uint64_t>(50ULL)));

    auto res = index->point_lookup(static_cast<uint64_t>(50ULL));
    EXPECT_FALSE(res.has_value());

}

// ------------------------------------------------------------
// Remove Non-Existing Key
// ------------------------------------------------------------
TEST_F(RangeLHTest, RemoveMissingKey) {
    EXPECT_FALSE(index->remove(static_cast<uint64_t>(999ULL)));
}
