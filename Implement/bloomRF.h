#ifndef BLOOM_RF_H
#define BLOOM_RF_H

#include <vector>
#include <stdexcept>
/*
 * Simple BloomRF (translated from Python)
 * - Bit-sequence keys (uint64_t)
 * - PMHF with Δ−1 word offset (as in your code)
 */

class BloomRF {
public:
    BloomRF(
        size_t n_items,
        double fp_prob,
        uint32_t d = 64,
        uint32_t delta = 4
    );

    void insert(uint64_t key);
    bool check_all_key(uint64_t key) const;
    bool check_at_level(uint64_t key, size_t i) const;
    size_t get_max_level() const;

    // ---------- Utils ----------
    size_t get_size() const;
    void print_configs() const;

private:
    // ---------- Size formula ----------
    static double cal_size(size_t n, double p, size_t word_bits);

    // ---------- Hashing (PMHF) ----------
    uint64_t hashing(uint64_t key, size_t i) const;

    // ---------- Bit operations ----------
    inline void set_bit(uint64_t pos);

    inline bool get_bit(uint64_t pos) const;

    // ---------- Hash constants ----------
    void init_hash_constants();

private:
    // parameters
    uint32_t d_;
    uint32_t delta_;
    size_t n_items;
    double fp_prob;
    size_t   m_;
    size_t   k_;

    // word layout
    uint64_t word_bits_;   // 2^(Δ-1)
    uint64_t word_mask_;   // 2^Δ - 1

    // storage
    std::vector<uint64_t> bits_;

    // hash constants
    std::vector<uint64_t> a_;
    std::vector<uint64_t> b_;
};

#endif // BLOOM_RF_H
