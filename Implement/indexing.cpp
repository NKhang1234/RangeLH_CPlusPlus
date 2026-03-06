#include "indexing.h"
#include <limits>

Storage::Storage(
    int master_init_num_bucket,
    double master_split_policy,
    int expected_n_items,
    double fp_prob_bloom_RF,
    int delta_bloom_RF,
    int key_length,
    int max_bytes_string,
    int float_scale
)
: index(
    master_init_num_bucket,
    master_split_policy,
    expected_n_items,
    fp_prob_bloom_RF,
    delta_bloom_RF,
    key_length,
    max_bytes_string,
    float_scale
)
{}

bool Storage::insert(uint64_t key, int record_id, const std::string& data)
{
    data_pool.emplace_back(record_id, data);

    Data* record = &data_pool.back();

    return index.insert(key, record);
}

std::optional<Data> Storage::get(uint64_t key)
{
    auto result = index.point_lookup(key);

    if (!result)
        return std::nullopt;

    const Data* record = *result;

    return *record;
}

double Storage::sum(uint64_t start, uint64_t end)
{
    auto results = index.range_lookup(start, end);

    if (!results)
        return 0.0;

    double total = 0.0;

    for (const Data* d : *results)
        total += d->getKey();

    return total;
}

double Storage::avg(uint64_t start, uint64_t end)
{
    auto results = index.range_lookup(start, end);

    if (!results || results->empty())
        return 0.0;

    double total = 0.0;

    for (const Data* d : *results)
        total += d->getKey();

    return total / results->size();
}

double Storage::minRange(uint64_t start, uint64_t end)
{
    auto results = index.range_lookup(start, end);

    if (!results || results->empty())
        return std::numeric_limits<double>::infinity();

    double min_val = (*results)[0]->getKey();

    for (const Data* d : *results)
        if (d->getKey() < min_val)
            min_val = d->getKey();

    return min_val;
}

double Storage::min() {
    auto head_opt = index.get_head();
    if (!head_opt)
        return std::numeric_limits<double>::infinity();

    return (*head_opt)->getKey();
}

double Storage::max() {
    auto tail_opt = index.get_tail();
    if (!tail_opt)
        return -std::numeric_limits<double>::infinity();

    return (*tail_opt)->getKey();
}

double Storage::maxRange(uint64_t start, uint64_t end)
{
    auto results = index.range_lookup(start, end);

    if (!results || results->empty())
        return -std::numeric_limits<double>::infinity();

    double max_val = (*results)[0]->getKey();

    for (const Data* d : *results)
        if (d->getKey() > max_val)
            max_val = d->getKey();

    return max_val;
}