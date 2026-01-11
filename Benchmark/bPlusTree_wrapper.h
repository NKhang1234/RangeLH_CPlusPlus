#pragma once

#include "index_interface.h"
#include "bPlusTree.h"  
#include <vector>
#include <cstddef>

/* ================================
   B+ Tree Wrapper
   ================================ */

template<typename K>
class BPlusTreeIndex : public Index<K> {
public:
    explicit BPlusTreeIndex(int order = 3)
        : tree_(order) {}

    /* ----------------------------
       Insert
       ---------------------------- */
    void insert(const K& key, const Data* data) override {
        tree_.insert(data);
    }

    /* ----------------------------
       Remove
       ---------------------------- */
    bool remove(const K& key) override {
        return tree_.remove(key);
    }

    /* ----------------------------
       Point Lookup
       ---------------------------- */
    bool find(const K& key, Data*& data) const override {
        data = tree_.search(key);
        return data != nullptr;
    }

    /* ----------------------------
       Range Query
       ---------------------------- */
    size_t range_query(const K& start, const K& end,
                        std::vector<Data*>& out) const override {
        auto results = tree_.rangeSearch(start, end);
        out.clear();
        out.insert(out.end(), results.begin(), results.end());
        
        return results.size();
    }

private:
    BPlusTree tree_;
};
