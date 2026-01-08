#include <gtest/gtest.h>
#include "linearHashing.h"
#include "worker.h"
#include <random>
#include <unordered_set>
#include <algorithm>

class LinearHashingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create some test workers
        for (int i = 0; i < 10; ++i) {
            testDatas.push_back( new Data(i, "Data" + std::to_string(i)));
        }
    }

    void TearDown() override {
        // Clean up test workers
        for (size_t i = 0; i < testDatas.size(); ++i) {
            if (testDatas[i] != nullptr) {
                delete testDatas[i];
                testDatas[i] = nullptr; // Crucial: prevent double deletion
            }
        }
        testDatas.clear();
    }

    std::vector<Data*> testDatas;
};

// ========== Constructor Tests ==========

TEST_F(LinearHashingTest, ConstructorDefault) {
    EXPECT_NO_THROW({
        LinearHashing lh;
    });
}

TEST_F(LinearHashingTest, ConstructorWithParameters) {
    EXPECT_NO_THROW({
        LinearHashing lh(32, 0.8);
    });
}

TEST_F(LinearHashingTest, ConstructorInitializesCorrectly) {
    LinearHashing lh(16, 0.75);
    
    EXPECT_EQ(lh.size(), 0);
    EXPECT_EQ(lh.bucketCount(), 16);
}

TEST_F(LinearHashingTest, ConstructorRoundsUpToPowerOfTwo) {
    // Test non-power-of-2 initial size
    LinearHashing lh(20, 0.75);  // Should round up to 32
    EXPECT_EQ(lh.bucketCount(), 32);
}

TEST_F(LinearHashingTest, ConstructorPowerOfTwoRemainsSame) {
    LinearHashing lh(16, 0.75);  // Already power of 2
    EXPECT_EQ(lh.bucketCount(), 16);
}

// ========== Single Insert and Lookup Tests ==========

TEST_F(LinearHashingTest, InsertAndLookupSingleWorker) {
    LinearHashing lh;
    uint64_t id = 42;
    Worker* worker = new Worker(id, testDatas[0]);
    lh.insert(id, worker);
    
    Worker* found = lh.lookup(id);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getID(), worker->getID());
}

TEST_F(LinearHashingTest, LookupNonExistentWorker) {
    LinearHashing lh;
    
    Worker* found = lh.lookup(999);
    EXPECT_EQ(found, nullptr);
}

TEST_F(LinearHashingTest, InsertMultipleWorkers) {
    LinearHashing lh;
    
    for (size_t i = 0; i < testDatas.size(); ++i) {
        Worker* worker = new Worker(i, testDatas[i]);
        lh.insert(i, worker);
    }
    
    EXPECT_EQ(lh.size(), testDatas.size());
    
    // Verify all can be found
    for (size_t i = 0; i < testDatas.size(); ++i) {
        Worker* found = lh.lookup(i);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->getID(), i);
    }
}

// ========== Remove Tests ==========

TEST_F(LinearHashingTest, RemoveExistingWorker) {
    LinearHashing lh;
    uint64_t id = 42;
    Worker* worker = new Worker(id, testDatas[0]);
    lh.insert(id, worker);
    EXPECT_EQ(lh.size(), 1);
    
    bool removed = lh.remove(id);
    EXPECT_TRUE(removed);
    EXPECT_EQ(lh.size(), 0);
    
    Worker* found = lh.lookup(id);
    EXPECT_EQ(found, nullptr);
}

TEST_F(LinearHashingTest, RemoveNonExistentWorker) {
    LinearHashing lh;
    
    bool removed = lh.remove(999);
    EXPECT_FALSE(removed);
}

TEST_F(LinearHashingTest, RemoveMultipleWorkers) {
    LinearHashing lh;
    std::vector<uint64_t> ids = {10, 20, 30, 40, 50};
    
    // Insert workers
    for (size_t i = 0; i < ids.size(); ++i) {
        lh.insert(ids[i], new Worker(ids[i], testDatas[i]));
    }
    
    EXPECT_EQ(lh.size(), ids.size());
    
    // Remove some workers
    EXPECT_TRUE(lh.remove(20));
    EXPECT_TRUE(lh.remove(40));
    
    EXPECT_EQ(lh.size(), 3);
    
    // Verify removed workers are gone
    EXPECT_EQ(lh.lookup(20), nullptr);
    EXPECT_EQ(lh.lookup(40), nullptr);
    
    // Verify remaining workers are still there
    EXPECT_NE(lh.lookup(10), nullptr);
    EXPECT_NE(lh.lookup(30), nullptr);
    EXPECT_NE(lh.lookup(50), nullptr);
}

// ========== Split Tests ==========

TEST_F(LinearHashingTest, SplitTriggersOnLoadFactor) {
    LinearHashing lh(4, 0.75);  // Small initial size, will split at 3 records
    std::vector<Data*> testDatasLocal;
    
    uint32_t initialBuckets = lh.bucketCount();
    
    // Insert enough to trigger split
    for (uint64_t i = 0; i < 10; ++i) {
        Data* data = new Data(i, "Worker" + std::to_string(i));
        testDatasLocal.push_back(data);
        Worker* w = new Worker(i, data);
        lh.insert(i, w);
    }
    
    // Should have more buckets now
    EXPECT_GT(lh.bucketCount(), initialBuckets);
    
    // All workers should still be findable
    for (uint64_t i = 0; i < 10; ++i) {
        EXPECT_NE(lh.lookup(i), nullptr) << "Failed to find worker " << i;
    }
    
    // Cleanup
    for(Data* data : testDatasLocal) {
        delete data;
    }
}

TEST_F(LinearHashingTest, DataIntegrityAfterSplit) {
    LinearHashing lh(8, 0.75);
    std::vector<uint64_t> ids;
    std::vector<Worker*> workers;
    std::vector<Data*> testDatasLocal;
    
    // Insert many workers to trigger multiple splits
    for (uint64_t i = 0; i < 100; ++i) {
        Data* data = new Data(i, "Worker" + std::to_string(i));
        Worker* w = new Worker(i, data);
        ids.push_back(i);
        testDatasLocal.push_back(data);
        lh.insert(i, w);
    }
    
    // Verify all workers are still accessible
    for (uint64_t i = 0; i < 100; ++i) {
        Worker* found = lh.lookup(i);
        ASSERT_NE(found, nullptr) << "Worker " << i << " not found after splits";
        EXPECT_EQ(found->getID(), i);
    }
    
    // Cleanup
    for(Data* data : testDatasLocal) {
        delete data;
    }
}
// ========== Batch Insert Tests ==========

TEST_F(LinearHashingTest, BatchInsert) {
    LinearHashing lh;
    std::vector<std::pair<uint64_t, Worker*>> entries;
    
    for (size_t i = 0; i < testDatas.size(); ++i) {
        entries.push_back({i * 10, new Worker(i * 10, testDatas[i])});
    }
    
    lh.batchInsert(entries);
    
    EXPECT_EQ(lh.size(), entries.size());
    
    // Verify all inserted
    for (const auto& entry : entries) {
        Worker* found = lh.lookup(entry.first);
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(found->getID(), entry.second->getID());
    }
}

TEST_F(LinearHashingTest, BatchInsertEmpty) {
    LinearHashing lh;
    std::vector<std::pair<uint64_t, Worker*>> entries;
    
    EXPECT_NO_THROW({
        lh.batchInsert(entries);
    });
    
    EXPECT_EQ(lh.size(), 0);
}

// ========== Edge Cases ==========

TEST_F(LinearHashingTest, InsertWithZeroID) {
    LinearHashing lh;
    uint64_t id = 0;
    
    Worker* worker = new Worker(id, testDatas[0]);
    lh.insert(id, worker);
    
    Worker* found = lh.lookup(id);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getID(), worker->getID());
}

TEST_F(LinearHashingTest, InsertWithMaxID) {
    LinearHashing lh;
    uint64_t id = UINT64_MAX;
    
    Worker* worker = new Worker(id, testDatas[0]);
    lh.insert(id, worker);
    
    Worker* found = lh.lookup(id);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getID(), worker->getID());
}

TEST_F(LinearHashingTest, InsertConsecutiveIDs) {
    LinearHashing lh;
    std::vector<Data*> testDatasLocal;
    
    for (uint64_t i = 0; i < 100; ++i) {
        Data* data = new Data(i, "Worker" + std::to_string(i));
        testDatasLocal.push_back(data);
        Worker* w = new Worker(i, data);
        lh.insert(i, w);
    }
    
    EXPECT_EQ(lh.size(), 100);
    
    // Verify all consecutive IDs
    for (uint64_t i = 0; i < 100; ++i) {
        EXPECT_NE(lh.lookup(i), nullptr) << "Failed at ID " << i;
    }
    
    // Cleanup
    for(Data* data : testDatasLocal) {
        delete data;
    }
}

TEST_F(LinearHashingTest, InsertSparseIDs) {
    LinearHashing lh;
    std::vector<uint64_t> sparseIds = {
        1ULL, 1000ULL, 1000000ULL, 
        1000000000ULL, 1000000000000ULL
    };
    
    for (size_t i = 0; i < sparseIds.size() && i < testDatas.size(); ++i) {
        lh.insert(sparseIds[i], new Worker(sparseIds[i], testDatas[i]));
    }
    
    // Verify all sparse IDs
    for (size_t i = 0; i < sparseIds.size() && i < testDatas.size(); ++i) {
        Worker* found = lh.lookup(sparseIds[i]);
        ASSERT_NE(found, nullptr) << "Failed at sparse ID " << sparseIds[i];
    }
}

// ========== Hash Collision Tests ==========

TEST_F(LinearHashingTest, HandleHashCollisions) {
    LinearHashing lh(4, 0.75);  // Small size to force collisions
    
    // Insert IDs that will likely collide
    std::vector<uint64_t> ids = {0, 4, 8, 12, 16, 20};  // Multiples of bucket size
    std::vector<Data*> testDatasLocal;
    
    for (size_t i = 0; i < ids.size(); ++i) {
        Data* data = new Data(i, "Worker" + std::to_string(i));
        Worker* w = new Worker(ids[i], data);
        testDatasLocal.push_back(data);
        lh.insert(ids[i], w);
    }
    
    // All should be findable despite collisions
    for (uint64_t id : ids) {
        EXPECT_NE(lh.lookup(id), nullptr) << "Failed to find ID " << id;
    }
    
    // Cleanup
    for (Data* data : testDatasLocal) {
        delete data;
    }
}

// ========== Stress Tests ==========

TEST_F(LinearHashingTest, StressTestManyInserts) {
    LinearHashing lh(16, 0.75);
    std::vector<Data*> testDatasLocal;
    const size_t numWorkers = 10000;
    
    // Insert many workers
    for (size_t i = 0; i < numWorkers; ++i) {
        Data* data = new Data(i, "Worker" + std::to_string(i));
        testDatasLocal.push_back(data);
        Worker* w = new Worker(i, data);
        lh.insert(i, w);
    }
    
    EXPECT_EQ(lh.size(), numWorkers);
    
    // Random sampling verification
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<size_t> dist(0, numWorkers - 1);
    
    for (int i = 0; i < 1000; ++i) {
        size_t idx = dist(rng);
        Worker* found = lh.lookup(idx);
        ASSERT_NE(found, nullptr) << "Failed at index " << idx;
        EXPECT_EQ(found->getID(), idx);
    }
    
    // Cleanup
    for(Data* data : testDatasLocal) {
        delete data;
    }
}

TEST_F(LinearHashingTest, StressTestRandomIDs) {
    LinearHashing lh;
    std::unordered_set<uint64_t> insertedIds;
    std::vector<Data*> testDatasLocal;
    const size_t numWorkers = 5000;
    
    std::mt19937_64 rng(67890);
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    
    // Insert random IDs
    for (size_t i = 0; i < numWorkers; ++i) {
        uint64_t id = dist(rng);
        
        // Avoid duplicates
        while (insertedIds.count(id)) {
            id = dist(rng);
        }
        
        insertedIds.insert(id);
        Data* data = new Data(i, "Worker" + std::to_string(i));
        Worker* w = new Worker(id, data);
        testDatasLocal.push_back(data);
        lh.insert(id, w);
    }
    
    EXPECT_EQ(lh.size(), numWorkers);
    
    // Verify all inserted IDs
    for (uint64_t id : insertedIds) {
        EXPECT_NE(lh.lookup(id), nullptr) << "Failed to find ID " << id;
    }
    
    // Cleanup
    for(Data* data : testDatasLocal) {
        delete data;
    }
}

TEST_F(LinearHashingTest, StressTestInsertRemoveCycles) {
    LinearHashing lh;
    std::vector<Data*> testDatasLocal;
    const size_t cycleSize = 1000;
    
    // Multiple insert/remove cycles
    for (int cycle = 0; cycle < 5; ++cycle) {
        // Insert
        for (size_t i = 0; i < cycleSize; ++i) {
            uint64_t id = cycle * cycleSize + i;
            Data* data = new Data(i, "Worker" + std::to_string(i));
            testDatasLocal.push_back(data);
            Worker* w = new Worker(id, data);
            lh.insert(id, w);
        }
        
        // Verify
        for (size_t i = 0; i < cycleSize; ++i) {
            uint64_t id = cycle * cycleSize + i;
            EXPECT_NE(lh.lookup(id), nullptr);
        }
        
        // Remove half
        for (size_t i = 0; i < cycleSize / 2; ++i) {
            uint64_t id = cycle * cycleSize + i;
            Worker* w = lh.lookup(id);
            EXPECT_TRUE(lh.remove(id));
            // delete w;
        }
    }
    
    // Cleanup remaining
    for (int cycle = 0; cycle < 5; ++cycle) {
        for (size_t i = cycleSize / 2; i < cycleSize; ++i) {
            uint64_t id = cycle * cycleSize + i;
            Worker* w = lh.lookup(id);
            if (w) {
                lh.remove(id);
                // delete w;
            }
        }
    }
    for (Data* data : testDatasLocal) {
        delete data;
    }
}

// ========== Load Factor Tests ==========

TEST_F(LinearHashingTest, LoadFactorStaysWithinBounds) {
    LinearHashing lh(8, 0.75);
    std::vector<Data*> testDatasLocal;
    
    for (size_t i = 0; i < 100; ++i) {
        Data* data = new Data(i, "Worker" + std::to_string(i));
        testDatasLocal.push_back(data);
        Worker* w = new Worker(i, data);
        lh.insert(i, w);
        
        double loadFactor = static_cast<double>(lh.size()) / lh.bucketCount();
        
        // Load factor should never exceed maxLoadFactor significantly
        EXPECT_LE(loadFactor, 0.75 * 1.5)  // Allow some tolerance
            << "Load factor too high at iteration " << i 
            << ": " << loadFactor;
    }
    
    // Cleanup
    for (Data* data : testDatasLocal) {
        delete data;
    }
}

TEST_F(LinearHashingTest, DifferentLoadFactors) {
    std::vector<double> loadFactors = {0.5, 0.75, 0.9};
    std::vector<Data*> testDatasLocal;
    
    for (double lf : loadFactors) {
        LinearHashing lh(8, lf);
        
        for (size_t i = 0; i < 50; ++i) {
            Data* data = new Data(i, "Data" + std::to_string(i));
            testDatasLocal.push_back(data);
            Worker* w = new Worker(i, data);
            lh.insert(i, w);
        }
        
        // All should be findable
        for (size_t i = 0; i < 50; ++i) {
            EXPECT_NE(lh.lookup(i), nullptr) 
                << "Failed with load factor " << lf;
        } 
    }
    // Cleanup
    for (Data* data : testDatasLocal) {
        delete data;
    }
}

// ========== Statistics Tests ==========

TEST_F(LinearHashingTest, PrintStatsNoThrow) {
    LinearHashing lh;
    
    // Insert some data
    for (size_t i = 0; i < testDatas.size(); ++i) {
        lh.insert(i, new Worker(i, testDatas[i]));
    }
    
    // Should not throw
    EXPECT_NO_THROW({
        testing::internal::CaptureStdout();
        lh.printStats();
        std::string output = testing::internal::GetCapturedStdout();
        EXPECT_FALSE(output.empty());
    });
}

TEST_F(LinearHashingTest, SizeAndBucketCount) {
    LinearHashing lh(16, 0.75);
    
    EXPECT_EQ(lh.size(), 0);
    EXPECT_EQ(lh.bucketCount(), 16);
    
    for (size_t i = 0; i < 5; ++i) {
        lh.insert(i, new Worker(i, testDatas[i]));
    }
    
    EXPECT_EQ(lh.size(), 5);
    EXPECT_GE(lh.bucketCount(), 16);
}

// ========== Performance Characteristic Tests ==========

TEST_F(LinearHashingTest, VerifyNoFalseNegatives) {
    LinearHashing lh;
    std::unordered_set<uint64_t> insertedIds;
    std::vector<Data*> testDatasLocal;
    
    // Insert 1000 workers
    for (size_t i = 0; i < 1000; ++i) {
        uint64_t id = i * 7;  // Use stride to spread IDs
        insertedIds.insert(id);
        Data* data = new Data(i, "Worker" + std::to_string(i));
        testDatasLocal.push_back(data);
        Worker* w = new Worker(id, data);
        lh.insert(id, w);
    }
    
    // Every inserted ID must be found (no false negatives)
    for (uint64_t id : insertedIds) {
        EXPECT_NE(lh.lookup(id), nullptr) 
            << "False negative for ID " << id;
    }
    
    // Cleanup
    for (Data* data : testDatasLocal) {
        delete data;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}