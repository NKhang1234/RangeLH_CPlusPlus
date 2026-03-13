#include <gtest/gtest.h>
#include "storage.h"

// Helper function to create storage with default parameters
Storage createStorage() {
    return Storage(
        4,      // master_init_num_bucket
        0.75,   // master_split_policy
        100000,   // expected_n_items
        0.01,   // fp_prob_bloom_RF
        2,      // delta_bloom_RF
        64,      // key_length
        8,    // max_bytes_string
        1       // float_scale
    );
}

// ---------------- INSERT & GET ----------------

TEST(StorageTest, Insert1) {
    Storage storage = createStorage();

    ASSERT_TRUE(storage.insert(10, "abc"));

    auto result = storage.get(10);

    ASSERT_TRUE(result.has_value());
}

TEST(StorageTest, Insert2) {
    Storage storage = createStorage();

    ASSERT_TRUE(storage.insert(9, "abc"));
    ASSERT_TRUE(storage.insert(20, "abc"));
    ASSERT_TRUE(storage.insert(1, "abc"));
    ASSERT_TRUE(storage.insert(7, "abc"));

    auto result = storage.get(1);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value().getKey() == 1);
}

// ---------------- REMOVE ----------------

TEST(StorageTest, RemoveRecord) {
    Storage storage = createStorage();

    storage.insert(10, "abc");

    ASSERT_TRUE(storage.remove(10));

    auto result = storage.get(10);

    ASSERT_FALSE(result.has_value());
}

TEST(StorageTest, RemoveRecord2) {
    Storage storage = createStorage();

    storage.insert(1, "abc");
    storage.insert(6, "abc");
    storage.insert(9, "abc");
    storage.insert(20, "abc");
    storage.insert(7, "abc");

    ASSERT_TRUE(storage.remove(6));

    auto result = storage.get(6);

    ASSERT_FALSE(result.has_value());
}

// ---------------- GLOBAL MIN / MAX ----------------

TEST(StorageTest, GlobalMinMax) {
    Storage storage = createStorage();

    storage.insert(5, "abc");
    storage.insert(12, "abc");
    storage.insert(1, "abc");
    storage.insert(9, "abc");
    storage.insert(15, "abc");
    storage.insert(26, "abc");

    EXPECT_DOUBLE_EQ(storage.min(), 1);
    EXPECT_DOUBLE_EQ(storage.max(), 26);
}

// ---------------- RANGE MIN / MAX ----------------

TEST(StorageTest, RangeMinMax) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");
    storage.insert(12, "abc");
    storage.insert(59, "abc");
    storage.insert(11, "abc");
    storage.insert(89, "abc");
    storage.insert(43, "abc");
    storage.insert(76, "abc");

    EXPECT_DOUBLE_EQ(storage.minRange(10, 45), 11);
    EXPECT_DOUBLE_EQ(storage.maxRange(10, 45), 43);
}

// ---------------- RANGE SUM ----------------

TEST(StorageTest, RangeSum) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");
    storage.insert(12, "abc");
    storage.insert(59, "abc");
    storage.insert(11, "abc");
    storage.insert(89, "abc");
    storage.insert(43, "abc");
    storage.insert(76, "abc");

    EXPECT_DOUBLE_EQ(storage.sumRange(10, 45), 94);
}

// ---------------- GLOBAL SUM ----------------

TEST(StorageTest, GlobalSum) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");
    storage.insert(12, "abc");
    storage.insert(59, "abc");
    storage.insert(11, "abc");
    storage.insert(89, "abc");
    storage.insert(43, "abc");
    storage.insert(76, "abc");

    EXPECT_DOUBLE_EQ(storage.sum(), 324);
}

// ---------------- RANGE AVERAGE ----------------

TEST(StorageTest, RangeAverage) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");//
    storage.insert(12, "abc");//
    storage.insert(59, "abc");
    storage.insert(11, "abc");//
    storage.insert(89, "abc");
    storage.insert(43, "abc");//
    storage.insert(76, "abc");

    EXPECT_DOUBLE_EQ(storage.avgRange(10, 45), 23.5);
}

// ---------------- GLOBAL AVERAGE ----------------

TEST(StorageTest, GlobalAverage) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");
    storage.insert(12, "abc");
    storage.insert(59, "abc");
    storage.insert(11, "abc");
    storage.insert(89, "abc");
    storage.insert(43, "abc");
    storage.insert(76, "abc");

    EXPECT_DOUBLE_EQ(storage.avg(), 40.5);
}

// ---------------- RANGE COUNT ----------------

TEST(StorageTest, RangeCount) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");//
    storage.insert(12, "abc");//
    storage.insert(59, "abc");
    storage.insert(11, "abc");//
    storage.insert(89, "abc");
    storage.insert(43, "abc");//
    storage.insert(76, "abc");

    EXPECT_EQ(storage.countRange(10, 45), 4);
}

// ---------------- GLOBAL COUNT ----------------

TEST(StorageTest, GlobalCount) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");//
    storage.insert(12, "abc");//
    storage.insert(59, "abc");
    storage.insert(11, "abc");//
    storage.insert(89, "abc");
    storage.insert(43, "abc");//
    storage.insert(76, "abc");

    EXPECT_EQ(storage.count(), 8);
}

// ---------------- EMPTY RANGE ----------------

TEST(StorageTest, EmptyRange) {
    Storage storage = createStorage();

    storage.insert(6, "abc");
    storage.insert(28, "abc");
    storage.insert(12, "abc");
    storage.insert(59, "abc");
    storage.insert(11, "abc");
    storage.insert(89, "abc");
    storage.insert(43, "abc");
    storage.insert(76, "abc");

    EXPECT_DOUBLE_EQ(storage.sumRange(77, 80), 0);
}

// ---------------- MULTIPLE INSERT ----------------

TEST(StorageTest, MultipleInsertions) {
    Storage storage = createStorage();

    for (int i = 1; i <= 100; i++) {
        storage.insert(i, std::to_string(i));
    }

    EXPECT_DOUBLE_EQ(storage.min(), 1);
    EXPECT_DOUBLE_EQ(storage.max(), 100);
}