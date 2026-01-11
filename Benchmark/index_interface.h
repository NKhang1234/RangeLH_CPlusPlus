#pragma once
#include <vector>
#include <cstdint>
#include "data.h"

template<typename K>
struct Index {
    virtual void insert(const K&, const Data*) = 0;
    virtual bool remove(const K&) = 0;
    virtual bool find(const K& key, Data*&) const = 0;
    virtual size_t range_query(const K&, const K&,
                               std::vector<Data*>&) const = 0;

    virtual ~Index() = default;
};
