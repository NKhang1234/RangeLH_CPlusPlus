#pragma once
#include "index_interface.h"
#include <unordered_map>

template<typename K, typename V>
class LinearHashingIndex : public Index<K, V> {
public:
    void insert(const K& k, const V& v) override {
        table_[k] = v;
    }

    bool find(const K& k, V& v) override {
        auto it = table_.find(k);
        if (it == table_.end()) return false;
        v = it->second;
        return true;
    }

    // Placeholder: replace with your bucket-ordered range scan
    size_t range_query(const K& l, const K& r,
                       std::vector<V>& out) override {
        size_t cnt = 0;
        for (auto& [k, v] : table_) {
            if (k >= l && k <= r) {
                out.push_back(v);
                ++cnt;
            }
        }
        return cnt;
    }

    size_t size() const override {
        return table_.size();
    }

private:
    std::unordered_map<K, V> table_;
};
