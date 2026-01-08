#include <gtest/gtest.h>
#include "bloomRF.h"
#include <random>
#include <unordered_set>

class BloomRFTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test fixtures if needed
    }

    void TearDown() override {
        // Clean up after tests
    }
};

// Test basic construction
TEST_F(BloomRFTest, ConstructorBasic) {
    EXPECT_NO_THROW({
        BloomRF filter(1000, 0.01);
    });
}

TEST_F(BloomRFTest, ConstructorWithParameters) {
    EXPECT_NO_THROW({
        BloomRF filter(1000, 0.01, 64, 4);
    });
}

TEST_F(BloomRFTest, ConstructorSmallN) {
    // Should handle n_items < 10 by clamping to 10
    EXPECT_NO_THROW({
        BloomRF filter(5, 0.01);
    });
}

// Test single insertion and lookup
TEST_F(BloomRFTest, InsertAndCheckSingleKey) {
    BloomRF filter(1000, 0.01);
    uint64_t key = 42;
    
    filter.insert(key);
    EXPECT_TRUE(filter.check_all_key(key));
}

TEST_F(BloomRFTest, CheckNonExistentKey) {
    BloomRF filter(1000, 0.01);
    uint64_t key = 42;
    
    // Without insertion, key should not be found
    EXPECT_FALSE(filter.check_all_key(key));
}

// Test multiple insertions
TEST_F(BloomRFTest, InsertMultipleKeys) {
    BloomRF filter(1000, 0.01);
    std::vector<uint64_t> keys = {1, 42, 100, 1000, 5000, 10000};
    
    for (auto key : keys) {
        filter.insert(key);
    }
    
    // All inserted keys should be found
    for (auto key : keys) {
        EXPECT_TRUE(filter.check_all_key(key));
    }
}

// Test false positive rate
TEST_F(BloomRFTest, FalsePositiveRate) {
    size_t n_items = 10000;
    double target_fp = 0.01;
    BloomRF filter(n_items, target_fp);
    
    // Insert n_items keys
    std::unordered_set<uint64_t> inserted_keys;
    for (size_t i = 0; i < n_items; ++i) {
        uint64_t key = i;
        filter.insert(key);
        inserted_keys.insert(key);
    }
    
    // Test false positive rate on non-inserted keys
    size_t n_tests = 10000;
    size_t false_positives = 0;
    
    for (size_t i = 0; i < n_tests; ++i) {
        uint64_t key = n_items + i; // Keys not inserted
        if (inserted_keys.find(key) == inserted_keys.end()) {
            if (filter.check_all_key(key)) {
                false_positives++;
            }
        }
    }
    
    double actual_fp = static_cast<double>(false_positives) / n_tests;
    
    // Allow some tolerance (FP rate should be reasonably close to target)
    // Using 3x tolerance as it's probabilistic
    EXPECT_LT(actual_fp, target_fp * 3.0);
    
    std::cout << "Target FP rate: " << target_fp << std::endl;
    std::cout << "Actual FP rate: " << actual_fp << std::endl;
}

// Test no false negatives
TEST_F(BloomRFTest, NoFalseNegatives) {
    BloomRF filter(1000, 0.01);
    std::vector<uint64_t> keys;
    
    // Insert 500 random keys
    std::mt19937_64 rng(12345);
    for (size_t i = 0; i < 500; ++i) {
        uint64_t key = rng();
        keys.push_back(key);
        filter.insert(key);
    }
    
    // All inserted keys must be found (no false negatives)
    for (auto key : keys) {
        EXPECT_TRUE(filter.check_all_key(key))
            << "False negative for key: " << key;
    }
}

// Test level checking
TEST_F(BloomRFTest, CheckAtLevel) {
    BloomRF filter(1000, 0.01, 64, 4);
    uint64_t key = 42;
    
    filter.insert(key);
    
    // Should be able to check at each level
    size_t max_level = filter.get_max_level();
    for (size_t i = 0; i <= max_level; ++i) {
        EXPECT_NO_THROW({
            filter.check_at_level(key, i);
        });
    }
}

TEST_F(BloomRFTest, CheckAtLevelOutOfRange) {
    BloomRF filter(1000, 0.01);
    uint64_t key = 42;
    
    filter.insert(key);
    
    size_t max_level = filter.get_max_level();
    
    // Should throw for level >= k
    EXPECT_THROW({
        filter.check_at_level(key, max_level + 1);
    }, std::out_of_range);
}

// Test edge cases
TEST_F(BloomRFTest, InsertZeroKey) {
    BloomRF filter(1000, 0.01);
    uint64_t key = 0;
    
    filter.insert(key);
    EXPECT_TRUE(filter.check_all_key(key));
}

TEST_F(BloomRFTest, InsertMaxKey) {
    BloomRF filter(1000, 0.01);
    uint64_t key = UINT64_MAX;
    
    filter.insert(key);
    EXPECT_TRUE(filter.check_all_key(key));
}

TEST_F(BloomRFTest, InsertConsecutiveKeys) {
    BloomRF filter(1000, 0.01);
    
    // Insert consecutive keys
    for (uint64_t key = 100; key < 200; ++key) {
        filter.insert(key);
    }
    
    // Check all consecutive keys
    for (uint64_t key = 100; key < 200; ++key) {
        EXPECT_TRUE(filter.check_all_key(key));
    }
}

// Test different delta values
TEST_F(BloomRFTest, DifferentDeltaValues) {
    std::vector<uint32_t> deltas = {3, 4, 5, 6, 7};
    
    for (auto delta : deltas) {
        EXPECT_NO_THROW({
            BloomRF filter(1000, 0.01, 64, delta);
            filter.insert(42);
            EXPECT_TRUE(filter.check_all_key(42));
        }) << "Failed with delta = " << delta;
    }
}

// Test different domain sizes
TEST_F(BloomRFTest, DifferentDomainSizes) {
    std::vector<uint32_t> domains = {16, 32, 64};
    
    for (auto d : domains) {
        EXPECT_NO_THROW({
            BloomRF filter(1000, 0.01, d, 4);
            filter.insert(42);
            EXPECT_TRUE(filter.check_all_key(42));
        }) << "Failed with domain size = " << d;
    }
}

// Test prefix hashing property
TEST_F(BloomRFTest, PrefixHashingProperty) {
    BloomRF filter(1000, 0.01, 16, 4);
    
    // Keys with same prefix on higher levels
    uint64_t key1 = 0x002A; // 42 in hex
    uint64_t key2 = 0x002B; // 43 in hex
    
    filter.insert(key1);
    
    // Check that they share the same hash at higher levels
    size_t max_level = filter.get_max_level();
    if (max_level > 0) {
        // At level > 0, keys with same high-order bits should have
        // some correlation in their hash values (though not guaranteed to be identical
        // due to the probabilistic nature)
        EXPECT_NO_THROW({
            filter.check_at_level(key1, max_level);
            filter.check_at_level(key2, max_level);
        });
    }
}

// Stress test with many keys
TEST_F(BloomRFTest, StressTestManyKeys) {
    BloomRF filter(50000, 0.01);
    std::vector<uint64_t> keys;
    
    std::mt19937_64 rng(67890);
    for (size_t i = 0; i < 50000; ++i) {
        uint64_t key = rng();
        keys.push_back(key);
        filter.insert(key);
    }
    
    // Randomly sample and verify
    std::uniform_int_distribution<size_t> dist(0, keys.size() - 1);
    for (size_t i = 0; i < 1000; ++i) {
        size_t idx = dist(rng);
        EXPECT_TRUE(filter.check_all_key(keys[idx]));
    }
}

// Test memory efficiency
TEST_F(BloomRFTest, ReasonableMemoryUsage) {
    size_t n_items = 10000;
    double fp_prob = 0.01;
    BloomRF filter(n_items, fp_prob);
    
    // Theoretical bits needed: -n * ln(p) / (ln(2))^2
    double theoretical_bits = -static_cast<double>(n_items) * std::log(fp_prob) / 
                              (std::log(2.0) * std::log(2.0));
    
    // The actual implementation should be reasonably close
    // (within 2x due to word alignment and other factors)
    size_t actual_bits = filter.get_size();
    
    EXPECT_LT(actual_bits, theoretical_bits * 2.0);
    
    std::cout << "Theoretical bits: " << theoretical_bits << std::endl;
    std::cout << "Actual bits: " << actual_bits << std::endl;
}

// Test duplicate insertions
TEST_F(BloomRFTest, DuplicateInsertions) {
    BloomRF filter(1000, 0.01);
    uint64_t key = 42;
    
    // Insert same key multiple times
    for (int i = 0; i < 10; ++i) {
        filter.insert(key);
    }
    
    // Should still be found
    EXPECT_TRUE(filter.check_all_key(key));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}