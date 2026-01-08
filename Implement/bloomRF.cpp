#include <vector>
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "bloomRF.h"

/*
 * Simple BloomRF (translated from Python)
 * - Bit-sequence keys (uint64_t)
 * - PMHF with Δ−1 word offset (as in your code)
 */


BloomRF::BloomRF(
    size_t n_items,
    double fp_prob,
    uint32_t d,
    uint32_t delta
): n_items(n_items), fp_prob(fp_prob), d_(d), delta_(delta)
{
    n_items = std::max(n_items, (size_t)10);

    // number of hash functions / levels
    k_ = static_cast<size_t>(
        std::ceil((double)(d_ - std::log2((double)n_items)) / delta_)
    );

    // word size = 2^(Δ-1)
    word_bits_ = 1ULL << (delta_ - 1);
    // word mask = 2^Δ - 1
    word_mask_ = (1ULL << delta_) - 1;

    m_ = static_cast<size_t>(cal_size(n_items, fp_prob, word_bits_));

    bits_.resize((m_ + 63) / 64, 0);

    init_hash_constants();
}

// ---------- Insert ----------
void BloomRF::insert(uint64_t key) {
    for (size_t i = 0; i < k_; ++i) {
        set_bit(hashing(key, i));
    }
}

// ---------- Point query ----------
bool BloomRF::check_all_key(uint64_t key) const {
    for (size_t i = 0; i < k_; ++i) {
        if (!get_bit(hashing(key, i)))
            return false;
    }
    return true;
}

bool BloomRF::check_at_level(uint64_t key, size_t i) const {
    if (i >= k_)
        throw std::out_of_range("Level i must be < k");
    return get_bit(hashing(key, i));
}

size_t BloomRF::get_max_level() const {
    return k_ - 1;
}

size_t BloomRF::get_size() const {
    return m_;
}

void BloomRF::print_configs() const {
    std::cout << "--- BloomRF Configuration Debug ---" << std::endl;
    std::cout << "d (Total depth/bits):   " << d_ << std::endl;
    std::cout << "delta (Word size bits): " << delta_ << std::endl;
    std::cout << "n_items (Expected):     " << n_items << std::endl;
    std::cout << "fp_prob (Target):       " << fp_prob << std::endl;
    std::cout << "m_ (Total bits):        " << m_ << std::endl;
    std::cout << "k_ (Hash functions):    " << k_ << std::endl;
    std::cout << "word_bits_:             " << word_bits_ << std::endl;
    std::cout << "word_mask_:             " << std::hex << "0x" << word_mask_ << std::dec << std::endl;
    std::cout << "bits_ vector size:      " << bits_.size() << " (uint64_t words)" << std::endl;
    std::cout << "Hash constants (a, b):  " << a_.size() << " pairs" << std::endl;
    std::cout << "Max Level:              " << get_max_level() << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

// ---------- Size formula ----------
double BloomRF::cal_size(size_t n, double p, size_t word_bits) {
    size_t raw_m =
    static_cast<size_t>(
        -(double)n * std::log(p) /
        (std::log(2.0) * std::log(2.0))
    );

    // round up to multiple of word_bits
    return ((raw_m + word_bits - 1) / word_bits) * word_bits;
}

// ---------- Hashing (PMHF) ----------
uint64_t BloomRF::hashing(uint64_t key, size_t i) const {
    uint64_t li = i * delta_;

    uint64_t prefix = key >> (li + delta_ - 1);
    uint64_t base =
        (a_[i] + b_[i] * prefix) %
        (m_ / word_bits_);

    uint64_t offset = (key >> li) & word_mask_;

    return (base << (delta_ - 1)) + offset;
}

// ---------- Bit operations ----------
inline void BloomRF::set_bit(uint64_t pos) {
    bits_[pos >> 6] |= (1ULL << (pos & 63));
}

inline bool BloomRF::get_bit(uint64_t pos) const {
    return bits_[pos >> 6] & (1ULL << (pos & 63));
}

// ---------- Hash constants ----------
void BloomRF::init_hash_constants() {
    static const uint64_t primes_a[] = {2, 3, 5, 7, 11, 13, 17, 19};
    static const uint64_t primes_b[] = {29, 31, 37, 41, 43, 47, 53, 59};

    size_t na = sizeof(primes_a) / sizeof(primes_a[0]);
    size_t nb = sizeof(primes_b) / sizeof(primes_b[0]);

    a_.resize(k_);
    b_.resize(k_);

    for (size_t i = 0; i < k_; ++i) {
        a_[i] = primes_a[i % na];
        b_[i] = primes_b[i % nb];
    }
}
