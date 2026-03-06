#ifndef STORAGE_H
#define STORAGE_H

#include <deque>
#include <optional>
#include <cstdint>
#include "rangeLH.h"
#include "data.h"

class Storage {
public:

    Storage(
        int master_init_num_bucket,
        double master_split_policy,
        int expected_n_items,
        double fp_prob_bloom_RF,
        int delta_bloom_RF,
        int key_length,
        int max_bytes_string,
        int float_scale
    );

    ~Storage() = default;

    // insert a record
    bool insert(uint64_t key, int record_id, const std::string& data);

    // point lookup
    std::optional<Data> get(uint64_t key);

    // aggregation
    double minRange(uint64_t start, uint64_t end);
    double maxRange(uint64_t start, uint64_t end);

    // Aggregate over the entire range
    double min(); 
    double max();

    double sum(uint64_t start, uint64_t end);
    double avg(uint64_t start, uint64_t end);

private:

    // storage owns data
    std::deque<Data> data_pool;

    // index
    RangeLH index;
};

#endif