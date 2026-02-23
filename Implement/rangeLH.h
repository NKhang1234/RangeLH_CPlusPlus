#ifndef RANGELH_H
#define RANGELH_H

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <worker.h>
#include <linearHashing.h>
#include <bloomRF.h>

class RangeLH {
private:
    std::unique_ptr<LinearHashing> master_LH;
    std::unique_ptr<BloomRF> bloom_RF;
    
    int num_worker;
    int key_length;
    int max_bytes_string;
    int float_scale;
    int delta_bloom_RF;
    Worker* worker_head;

public:
    RangeLH(int master_init_num_bucket, double master_split_policy,
            int expected_n_items, double fp_prob_bloom_RF, int delta_bloom_RF, int key_length,
            int max_bytes_string, int float_scale);
    
    ~RangeLH();

    // Core functions
    bool insert(uint64_t key, const Data* value);
    bool insert(double key, const Data* value);
    bool insert(const std::string& key, const Data* value);
    
    std::optional<const Data*> point_lookup(uint64_t key);
    std::optional<const Data*> point_lookup(double key);
    std::optional<const Data*> point_lookup(const std::string& key);

    std::optional<std::vector<const Data*>> range_lookup(uint64_t key_start, uint64_t key_end);
    std::optional<std::vector<const Data*>> range_lookup(double key_start, double key_end);
    std::optional<std::vector<const Data*>> range_lookup(const std::string& key_start, const std::string& key_end);
    std::optional<std::vector<const Data*>> pattern_lookup(const std::string& pattern_key);
    
    bool remove(uint64_t key);
    bool remove(double key);
    bool remove(const std::string& key);

    // Test utility
    void print_worker_chain();

private:
    // Utility functions
    uint64_t string_to_int_lex(const std::string& s);
    uint64_t float_to_scaled_int(double f);
    std::string next_lexicographic_string(const std::string& s);
    
    Worker* get_left_adjacent_worker(uint64_t worker_key);
    Worker* get_right_adjacent_worker(uint64_t worker_key);
    
    std::vector<std::pair<uint64_t, int>> get_left_children_list(uint64_t cur_key, int cur_level, uint64_t worker_key);
    std::vector<std::pair<uint64_t, int>> get_right_children_list(uint64_t cur_key, int cur_level, uint64_t worker_key);
    
    // Internal implementation
    bool insert_impl(uint64_t key, const Data* value);
    std::optional<const Data*> point_lookup_impl(uint64_t key);
    std::optional<std::vector<const Data*>> range_lookup_impl(uint64_t key_start, uint64_t key_end);
    bool remove_impl(uint64_t key);
};

#endif // RANGELH_H