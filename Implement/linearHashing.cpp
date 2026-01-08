#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <memory>
#include "worker.h"
#include "linearHashing.h"

LinearHashing::LinearHashing(uint32_t initialSize, double maxLoad) 
    : level(0), next(0), numRecords(0), 
        initialBuckets(initialSize), maxLoadFactor(maxLoad) {
    
    // Ensure initial size is power of 2 for optimization
    if ((initialSize & (initialSize - 1)) != 0) {
        // Round up to next power of 2
        initialSize--;
        initialSize |= initialSize >> 1;
        initialSize |= initialSize >> 2;
        initialSize |= initialSize >> 4;
        initialSize |= initialSize >> 8;
        initialSize |= initialSize >> 16;
        initialSize++;
        initialBuckets = initialSize;
    }
    
    buckets.reserve(initialSize * 2);  // Pre-allocate for growth
    for (uint32_t i = 0; i < initialSize; i++) {
        buckets.push_back(new Bucket());
    }
}

LinearHashing::~LinearHashing() {
    for (auto* bucket : buckets) {
        delete bucket;
    }
}

// Split operation - optimized for pointer manipulation
void LinearHashing::split() {
    // Allocate new bucket
    buckets.push_back(new Bucket());
    uint32_t newBucketIdx = buckets.size() - 1;
    
    // Rehash from bucket at 'next' position
    Bucket* oldBucket = buckets[next];
    Entry* curr = oldBucket->head;
    Entry* prev = nullptr;
    
    oldBucket->head = nullptr;
    Bucket* newBucket = buckets[newBucketIdx];
    
    while (curr) {
        Entry* nextEntry = curr->next;
        uint32_t newIdx = hash(curr->id, level + 1);
        
        if (newIdx == newBucketIdx) {
            // Move to new bucket
            curr->next = newBucket->head;
            newBucket->head = curr;
        } else {
            // Stay in old bucket
            curr->next = oldBucket->head;
            oldBucket->head = curr;
        }
        
        curr = nextEntry;
    }
    
    // Update split pointer
    next++;
    
    // Check for level increment
    if (next == (initialBuckets << level)) {
        level++;
        next = 0;
    }
}

// Insert worker with binary ID - optimized hot path
void LinearHashing::insert(uint64_t id, Worker* worker) {
    uint32_t idx = getBucketIndex(id);
    Bucket* bucket = buckets[idx];
    
    // Insert new entry at head (O(1))
    Entry* newEntry = new Entry(id, worker);
    newEntry->next = bucket->head;
    bucket->head = newEntry;
    
    numRecords++;
    checkAndSplit();
}

// Search for worker by binary ID - optimized lookup
Worker* LinearHashing::lookup(uint64_t id) const {
    uint32_t idx = getBucketIndex(id);
    Entry* curr = buckets[idx]->head;
    
    while (curr != nullptr) {
        if (curr->id == id) {
            return curr->worker;
        }
        curr = curr->next;
    }
    return nullptr;
}

// Remove worker by ID
bool LinearHashing::remove(uint64_t id) {
    uint32_t idx = getBucketIndex(id);
    Bucket* bucket = buckets[idx];
    
    Entry* curr = bucket->head;
    Entry* prev = nullptr;
    
    while (curr != nullptr) {
        if (curr->id == id) {
            if (prev) {
                prev->next = curr->next;
            } else {
                bucket->head = curr->next;
            }
            delete curr;
            curr = nullptr;
            numRecords--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

// Batch insert for better cache performance
void LinearHashing::batchInsert(const std::vector<std::pair<uint64_t, Worker*>>& entries) {
    for (const auto& entry : entries) {
        insert(entry.first, entry.second);
    }
}

// Get statistics
void LinearHashing::printStats() const {
    std::cout << "=== Linear Hashing Statistics ===" << std::endl;
    std::cout << "Total Workers: " << numRecords << std::endl;
    std::cout << "Total Buckets: " << buckets.size() << std::endl;
    std::cout << "Level: " << level << std::endl;
    std::cout << "Next Split: " << next << std::endl;
    std::cout << "Load Factor: " << static_cast<double>(numRecords) / buckets.size() << std::endl;
    std::cout << "Memory Usage: ~" << (buckets.size() * sizeof(Bucket*) + numRecords * sizeof(Entry)) / 1024 << " KB" << std::endl;
    
    // Analyze chain lengths
    uint32_t maxChain = 0;
    uint32_t totalChain = 0;
    uint32_t emptyBuckets = 0;
    
    for (const auto* bucket : buckets) {
        uint32_t chainLen = 0;
        Entry* curr = bucket->head;
        while (curr) {
            chainLen++;
            curr = curr->next;
        }
        
        if (chainLen == 0) emptyBuckets++;
        maxChain = std::max(maxChain, chainLen);
        totalChain += chainLen;
    }
    
    std::cout << "Max Chain Length: " << maxChain << std::endl;
    std::cout << "Avg Chain Length: " << static_cast<double>(totalChain) / buckets.size() << std::endl;
    std::cout << "Empty Buckets: " << emptyBuckets << std::endl;
}

uint32_t LinearHashing::size() const { return numRecords; }
uint32_t LinearHashing::bucketCount() const { return buckets.size(); }


