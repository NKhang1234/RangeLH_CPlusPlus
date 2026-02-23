#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <string>
#include <cstdint>

#include "data.h"
#include "bPlusTree.h"
#include "rangeLH.h"

class RangeLH_Config {
public:
    static constexpr int master_init_num_bucket = 4;
    static constexpr double master_split_policy = 0.75;
    static constexpr int expected_n_items = 10000000;
    static constexpr double fp_prob_bloom_RF = 0.01;
    static constexpr int delta_bloom_RF = 3;
    static constexpr int key_length = 64;
    static constexpr int max_bytes_string = 8;
    static constexpr int float_scale = 1000;
};

/* ================= DATA GENERATION ================= */

static std::vector<std::pair<uint64_t, Data*>>
generate_dataset(size_t N) {
    std::vector<std::pair<uint64_t, Data*>> dataset;
    dataset.reserve(N);

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<uint64_t> key_dist(1, 1e12);

    for (size_t i = 0; i < N; i++) {
        uint64_t key = key_dist(rng);

        // fixed-size string (avoid randomness affecting memory)
        std::string payload = "benchmark_payload_123456";

        dataset.emplace_back(
            key,
            new Data(static_cast<int>(i), payload)
        );
    }

    return dataset;
}

/* ================= INSERT BENCH ================= */

static void BM_BPlus_Insert(benchmark::State& state) {
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    for (auto _ : state) {
        state.PauseTiming();
        BPlusTree tree;
        state.ResumeTiming();

        for (auto& [key, data] : dataset)
            tree.insert(key, data);
    }
}
BENCHMARK(BM_BPlus_Insert)->Arg(100000)->Arg(1000000);

static void BM_RangeLH_Insert(benchmark::State& state) {
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    for (auto _ : state) {
        state.PauseTiming();
        RangeLH index(
            RangeLH_Config::master_init_num_bucket,
            RangeLH_Config::master_split_policy,
            RangeLH_Config::expected_n_items,
            RangeLH_Config::fp_prob_bloom_RF,
            RangeLH_Config::delta_bloom_RF,
            RangeLH_Config::key_length,
            RangeLH_Config::max_bytes_string,
            RangeLH_Config::float_scale
        );
        state.ResumeTiming();

        for (auto& [key, data] : dataset)
            index.insert(key, data);
    }
}
BENCHMARK(BM_RangeLH_Insert)->Arg(100000)->Arg(1000000);

/* ================= SEARCH BENCH ================= */

static void BM_BPlus_PointSearch(benchmark::State& state) {
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    BPlusTree tree;
    for (auto& [key, data] : dataset)
        tree.insert(key, data);

    for (auto _ : state) {
        for (auto& [key, _] : dataset)
            benchmark::DoNotOptimize(tree.search(key));
    }
}
BENCHMARK(BM_BPlus_PointSearch)->Arg(100000)->Arg(1000000);

static void BM_RangeLH_PointSearch(benchmark::State& state) {
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    RangeLH index(
        RangeLH_Config::master_init_num_bucket,
        RangeLH_Config::master_split_policy,
        RangeLH_Config::expected_n_items,
        RangeLH_Config::fp_prob_bloom_RF,
        RangeLH_Config::delta_bloom_RF,
        RangeLH_Config::key_length,
        RangeLH_Config::max_bytes_string,
        RangeLH_Config::float_scale
    );
    for (auto& [key, data] : dataset)
        index.insert(key, data);

    for (auto _ : state) {
        for (auto& [key, _] : dataset)
            benchmark::DoNotOptimize(index.point_lookup(key));
    }
}
BENCHMARK(BM_RangeLH_PointSearch)->Arg(100000)->Arg(1000000);

BENCHMARK_MAIN();