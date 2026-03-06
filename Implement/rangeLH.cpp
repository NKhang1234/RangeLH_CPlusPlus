#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <algorithm>
#include <cstring>
#include <stack>
#include "rangeLH.h"

// Implementation

RangeLH::RangeLH(int master_init_num_bucket, double master_split_policy,
                 int expected_n_items, double fp_prob_bloom_RF, int delta_bloom_RF, int key_length,
                int max_bytes_string, int float_scale)
    : num_worker(0),
      key_length(key_length),
      max_bytes_string(max_bytes_string),
      float_scale(float_scale),
      delta_bloom_RF(delta_bloom_RF),
      worker_head(nullptr),
      worker_tail(nullptr) {
    // Initialize master_LH and bloom_RF
        master_LH = std::make_unique<LinearHashing>(master_init_num_bucket, master_split_policy);
        bloom_RF = std::make_unique<BloomRF>(expected_n_items, fp_prob_bloom_RF, key_length, delta_bloom_RF);
}

RangeLH::~RangeLH() = default;

// ============================================================
// Utility Functions
// ============================================================

uint64_t RangeLH::string_to_int_lex(const std::string& s) {
    uint8_t buffer[8] = {0}; // Support up to 8 bytes for uint64_t
    size_t copy_len = std::min(s.length(), static_cast<size_t>(max_bytes_string));
    std::memcpy(buffer, s.c_str(), copy_len);
    
    // Convert to big-endian uint64_t
    uint64_t result = 0;
    for (int i = 0; i < max_bytes_string && i < 8; ++i) {
        result = (result << 8) | buffer[i];
    }
    return result;
}

uint64_t RangeLH::float_to_scaled_int(double f) {
    return static_cast<uint64_t>(f * float_scale);
}

std::string RangeLH::next_lexicographic_string(const std::string& s) {
    if (s.empty()) return s;
    
    std::string result = s;
    for (int i = static_cast<int>(result.length()) - 1; i >= 0; --i) {
        if (result[i] != static_cast<char>(0xFFFF)) {
            result[i] = static_cast<char>(result[i] + 1);
            return result.substr(0, i + 1);
        }
    }
    // All characters maxed out
    return s + '\x00';
}

Worker* RangeLH::get_left_adjacent_worker(uint64_t worker_key) {
    int max_level = bloom_RF->get_max_level();
    std::stack<std::pair<uint64_t, int>> search_stack;
    search_stack.push({0, max_level});
    
    while (!search_stack.empty()) {
        auto [key, level] = search_stack.top();
        search_stack.pop();
        
        if (level == 0) {
            Worker* prev_worker = master_LH->lookup(key);
            if (prev_worker != nullptr) {
                return prev_worker;
            }
            continue;
        }
        
        if (!bloom_RF->check_at_level(key, level)) {
            continue;
        }
        
        auto child_list = get_left_children_list(key, level, worker_key);
        for (const auto& child : child_list) {
            search_stack.push(child);
        }
    }
    return nullptr;
}

Worker* RangeLH::get_right_adjacent_worker(uint64_t worker_key) {
    int max_level = bloom_RF->get_max_level();
    std::stack<std::pair<uint64_t, int>> search_stack;
    search_stack.push({0, max_level});
    
    while (!search_stack.empty()) {
        auto [key, level] = search_stack.top();
        search_stack.pop();
        
        if (level == 0) {
            Worker* next_worker = master_LH->lookup(key);
            if (next_worker != nullptr) {
                return next_worker;
            }
            continue;
        }
        
        if (!bloom_RF->check_at_level(key, level)) {
            continue;
        }
        
        auto child_list = get_right_children_list(key, level, worker_key);
        for (const auto& child : child_list) {
            search_stack.push(child);
        }
    }
    return nullptr;
}

std::vector<std::pair<uint64_t, int>> RangeLH::get_left_children_list(
    uint64_t cur_key, int cur_level, uint64_t worker_key) {
    if (cur_level == 0) {
        return {};
    }

    uint64_t step = 1ULL << ((cur_level - 1) * delta_bloom_RF);
    uint64_t num_children = 1ULL << delta_bloom_RF;
    std::vector<std::pair<uint64_t, int>> children;
    
    for (uint64_t j = 0; j < num_children; ++j) {
        uint64_t child_key = cur_key + j * step;
        if (child_key < worker_key) {
            children.push_back({child_key, cur_level - 1});
        }
    }
    
    // Check if worker_key is in current interval
    if (cur_key <= worker_key && worker_key < cur_key + (num_children * step)) {
        uint64_t mask = ~((1ULL << ((cur_level - 1) * delta_bloom_RF)) - 1);
        uint64_t aligned_worker_key = worker_key & mask;
        children.push_back({aligned_worker_key, cur_level - 1});
    }
    
    return children;
}

std::vector<std::pair<uint64_t, int>> RangeLH::get_right_children_list(
    uint64_t cur_key, int cur_level, uint64_t worker_key) {
    if (cur_level == 0) {
        return {};
    }
    
    uint64_t step = 1ULL << ((cur_level - 1) * delta_bloom_RF);
    uint64_t num_children = 1ULL << delta_bloom_RF;
    std::vector<std::pair<uint64_t, int>> children;
    
    for (int64_t j = num_children - 1; j >= 0; --j) {
        uint64_t child_key = cur_key + j * step;
        if (child_key > worker_key) {
            children.push_back({child_key, cur_level - 1});
        }
    }
    
    // Check if worker_key is in current interval
    if (cur_key <= worker_key && worker_key < cur_key + (num_children * step)) {
        uint64_t mask = ~((1ULL << ((cur_level - 1) * delta_bloom_RF)) - 1);
        uint64_t aligned_worker_key = worker_key & mask;
        children.push_back({aligned_worker_key, cur_level - 1});
    }
    
    return children;
}

// ============================================================
// Test Utility
// ============================================================

void RangeLH::print_worker_chain() {
    Worker* current = worker_head;
    while (current != nullptr) {
        std::cout << "Worker ID: " << current->getID() << " Data: " << current->getData()->getData() << std::endl;
        current = current->getNext();
    }
}

// ============================================================
// Core Functions - Implementation
// ============================================================

bool RangeLH::insert_impl(uint64_t key, const Data* value) {
    uint64_t worker_key = key;

    Worker* existed_worker = master_LH->lookup(worker_key);
    if (existed_worker != nullptr) {
        if (!existed_worker->hasData()) {
            num_worker++;
        }
        return existed_worker->updateData(value);
    }
    
    // Create new worker
    Worker* new_worker = new Worker(worker_key, value);
    
    // Connect with adjacent workers
    Worker* prev_worker = get_left_adjacent_worker(worker_key);
    
    if (prev_worker != nullptr) {
        Worker* next_worker = prev_worker->getNext();
        prev_worker->setNext(new_worker);
        if (next_worker != nullptr) {
            new_worker->setNext(next_worker);
        } else {
            worker_tail = new_worker;
        }
    }
    else {
        new_worker->setNext(worker_head);
        worker_head = new_worker;
        worker_tail = (worker_tail == nullptr) ? new_worker : worker_tail;
    }
    
    master_LH->insert(worker_key, new_worker);
    bloom_RF->insert(worker_key);
    ++num_worker;
    
    return true;
}

std::optional<const Data*> RangeLH::point_lookup_impl(uint64_t key) {
    uint64_t worker_key = key;
    Worker* worker_ptr = master_LH->lookup(worker_key);
    
    if (worker_ptr != nullptr and worker_ptr->hasData()) {
        return worker_ptr->getData();
    }
    return std::nullopt;
}

std::optional<const Data*> RangeLH::get_head() {
    if (worker_head != nullptr && worker_head->hasData()) {
        return worker_head->getData();
    }
    return std::nullopt;
}

std::optional<const Data*> RangeLH::get_tail() {
    if (worker_tail != nullptr && worker_tail->hasData()) {
        return worker_tail->getData();
    }
    return std::nullopt;
}

std::optional<std::vector<const Data*>> RangeLH::range_lookup_impl(uint64_t key_start, uint64_t key_end) {
    uint64_t start_worker_key = key_start;
    Worker* start_worker = master_LH->lookup(start_worker_key);
    if (start_worker == nullptr) {
        start_worker = get_right_adjacent_worker(start_worker_key);
    }
    
    if (start_worker == nullptr) {
        return std::nullopt;
    }
    
    std::vector<const Data*> result;
    Worker* p_worker = start_worker;
    
    while (p_worker != nullptr) {
        if (p_worker->getID() > key_end) {
            break;
        }
        result.push_back(p_worker->getData());
        p_worker = p_worker->getNext();
    }
    
    if (result.empty()) {
        return std::nullopt;
    }
    return result;
}

bool RangeLH::remove_impl(uint64_t key) {
    
    uint64_t worker_key = key;
    Worker* worker_ptr = master_LH->lookup(worker_key);
    
    if (worker_ptr != nullptr) {
        return worker_ptr->deleteData();
    }

    --num_worker;
    return false;
}

// ============================================================
// Public Interface - Type Overloads
// ============================================================

bool RangeLH::insert(uint64_t key, const Data* value) {
    return insert_impl(key, value);
}

bool RangeLH::insert(double key, const Data* value) {
    return insert_impl(float_to_scaled_int(key), value);
}

bool RangeLH::insert(const std::string& key, const Data* value) {
    return insert_impl(string_to_int_lex(key), value);
}

std::optional<const Data*> RangeLH::point_lookup(uint64_t key) {
    return point_lookup_impl(key);
}

std::optional<const Data*> RangeLH::point_lookup(double key) {
    return point_lookup_impl(float_to_scaled_int(key));
}

std::optional<const Data*> RangeLH::point_lookup(const std::string& key) {
    return point_lookup_impl(string_to_int_lex(key));
}

std::optional<std::vector<const Data*>> RangeLH::range_lookup(uint64_t key_start, uint64_t key_end) {
    return range_lookup_impl(key_start, key_end);
}

std::optional<std::vector<const Data*>> RangeLH::range_lookup(double key_start, double key_end) {
    return range_lookup_impl(float_to_scaled_int(key_start), float_to_scaled_int(key_end));
}

std::optional<std::vector<const Data*>> RangeLH::range_lookup(const std::string& key_start, const std::string& key_end) {
    return range_lookup_impl(string_to_int_lex(key_start), string_to_int_lex(key_end));
}

std::optional<std::vector<const Data*>> RangeLH::pattern_lookup(const std::string& pattern_key) {
    if (pattern_key.empty() || pattern_key.back() != '%') {
        return std::nullopt;
    }
    
    std::string prefix = pattern_key.substr(0, pattern_key.length() - 1);
    if (prefix.find('%') != std::string::npos) {
        return std::nullopt;
    }
    
    return range_lookup(prefix, next_lexicographic_string(prefix));
}

bool RangeLH::remove(uint64_t key) {
    return remove_impl(key);
}

bool RangeLH::remove(double key) {
    return remove_impl(float_to_scaled_int(key));
}

bool RangeLH::remove(const std::string& key) {
    return remove_impl(string_to_int_lex(key));
}

