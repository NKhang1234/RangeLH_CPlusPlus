#ifndef LINEAR_HASHING_H
#define LINEAR_HASHING_H

#include <vector>
#include <cstdint>
#include "worker.h"

class LinearHashing {
private:
    // Optimized bucket entry for binary ID and pointer
    struct Entry {
        uint64_t id;        // Binary worker ID
        Worker* worker;     // Pointer to worker
        Entry* next;        // For overflow chaining
        
        Entry(uint64_t i, Worker* w) : id(i), worker(w), next(nullptr) {}
        ~Entry() {
            if (worker != nullptr) {
                delete worker;
            }
        }
    };

    // Bucket structure with inline storage
    struct Bucket {
        Entry* head;        // Head of overflow chain
        
        Bucket() : head(nullptr) {}
        
        ~Bucket() {
            Entry* curr = head;
            while (curr) {
                Entry* temp = curr;
                curr = curr->next;
                delete temp;
            }
        }
    };

    std::vector<Bucket*> buckets;
    uint32_t level;             // Current level
    uint32_t next;              // Next bucket to split
    uint32_t numRecords;        // Total records
    uint32_t initialBuckets;    // Initial N
    double maxLoadFactor;       // Split threshold
    
    // Fast hash for binary IDs - uses bit manipulation
    inline uint32_t hash(uint64_t id, uint32_t i) const {
        // For level i, mask is (N * 2^i) - 1
        uint32_t size = initialBuckets << i;
        return id & (size - 1);  // Equivalent to id % size when size is power of 2
    }

    // Get bucket index with optimized path
    inline uint32_t getBucketIndex(uint64_t id) const {
        uint32_t h0 = hash(id, level);
        
        // Check if we need to use next level hash
        if (h0 < next) {
            return hash(id, level + 1);
        }
        return h0;
    }

    // Split operation - optimized for pointer manipulation
    void split();

    // Check load factor and split if needed
    inline void checkAndSplit() {
        if (static_cast<double>(numRecords) / buckets.size() > maxLoadFactor) {
            split();
        }
    }

public:
    LinearHashing(uint32_t initialSize = 16, double maxLoad = 0.75);
    ~LinearHashing();

    // Insert worker with binary ID - optimized hot path
    void insert(uint64_t id, Worker* worker);

    // Search for worker by binary ID - optimized lookup
    Worker* lookup(uint64_t id) const;

    // Remove worker by ID
    bool remove(uint64_t id);

    // Batch insert for better cache performance
    void batchInsert(const std::vector<std::pair<uint64_t, Worker*>>& entries);

    // Get statistics
    void printStats() const;

    uint32_t size() const;
    uint32_t bucketCount() const;
};

#endif // LINEAR_HASHING_H
