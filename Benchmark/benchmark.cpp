#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <fstream>

#include "data.h"
#include "bPlusTree.h"
#include "rangeLH.h"

class RangeLH_Config
{
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

constexpr int MINSIZE = 1'000;
constexpr int MAXSIZE = 1'000'000;
constexpr int SIZEOFFSET = 10;

/* ================= DATA GENERATION ================= */

static std::vector<std::pair<uint64_t, Data *>>
generate_dataset(size_t N)
{
    std::vector<std::pair<uint64_t, Data *>> dataset;
    dataset.reserve(N);

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<uint64_t> key_dist(1, 1e12);

    for (size_t i = 0; i < N; i++)
    {
        uint64_t key = key_dist(rng);

        // fixed-size string (avoid randomness affecting memory)
        std::string payload = "benchmark_payload_123456";

        dataset.emplace_back(
            key,
            new Data(static_cast<int>(i), payload));
    }

    return dataset;
}

/* ================= RESOURCE CALCULATION ================= */

size_t getCurrentRSS()
{
    long rss = 0L;
    FILE *fp = nullptr;
    if ((fp = fopen("/proc/self/statm", "r")) == nullptr)
        return 0;
    if (fscanf(fp, "%*s%ld", &rss) != 1)
    {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return (size_t)rss * sysconf(_SC_PAGESIZE);
}

/* ================= INSERT BENCH ================= */

static void BM_BPlus_Insert(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    for (auto _ : state)
    {
        state.PauseTiming();
        BPlusTree tree;
        state.ResumeTiming();

        for (auto &[key, data] : dataset)
            tree.insert(key, data);
    }
}
BENCHMARK(BM_BPlus_Insert)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

static void BM_RangeLH_Insert(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    for (auto _ : state)
    {
        state.PauseTiming();
        RangeLH index(
            RangeLH_Config::master_init_num_bucket,
            RangeLH_Config::master_split_policy,
            N * SIZEOFFSET, // RangeLH_Config::expected_n_items,
            RangeLH_Config::fp_prob_bloom_RF,
            RangeLH_Config::delta_bloom_RF,
            RangeLH_Config::key_length,
            RangeLH_Config::max_bytes_string,
            RangeLH_Config::float_scale);
        state.ResumeTiming();

        for (auto &[key, data] : dataset)
            index.insert(key, data);
    }
}
BENCHMARK(BM_RangeLH_Insert)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

/* ================= DELETE BENCH ================= */
static void BM_BPlus_Delete(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    for (auto _ : state)
    {
        state.PauseTiming();
        BPlusTree tree;
        for (auto &[key, data] : dataset)
            tree.insert(key, data);

        state.ResumeTiming();

        for (auto &[key, _] : dataset)
            tree.remove(key); // or tree.delete(key)
    }
}
BENCHMARK(BM_BPlus_Delete)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

static void BM_RangeLH_Delete(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    for (auto _ : state)
    {
        state.PauseTiming();
        RangeLH index(
            RangeLH_Config::master_init_num_bucket,
            RangeLH_Config::master_split_policy,
            N * SIZEOFFSET, // RangeLH_Config::expected_n_items,
            RangeLH_Config::fp_prob_bloom_RF,
            RangeLH_Config::delta_bloom_RF,
            RangeLH_Config::key_length,
            RangeLH_Config::max_bytes_string,
            RangeLH_Config::float_scale);
        for (auto &[key, data] : dataset)
            index.insert(key, data);

        state.ResumeTiming();

        for (auto &[key, _] : dataset)
            index.remove(key);
    }
}
BENCHMARK(BM_RangeLH_Delete)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

/* ================= POINT SEARCH BENCH ================= */

static void BM_BPlus_PointSearch(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    BPlusTree tree;
    for (auto &[key, data] : dataset)
        tree.insert(key, data);

    for (auto _ : state)
    {
        for (auto &[key, _] : dataset)
            benchmark::DoNotOptimize(tree.search(key));
    }
}
BENCHMARK(BM_BPlus_PointSearch)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

static void BM_RangeLH_PointSearch(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);

    RangeLH index(
        RangeLH_Config::master_init_num_bucket,
        RangeLH_Config::master_split_policy,
        N * SIZEOFFSET, // RangeLH_Config::expected_n_items,
        RangeLH_Config::fp_prob_bloom_RF,
        RangeLH_Config::delta_bloom_RF,
        RangeLH_Config::key_length,
        RangeLH_Config::max_bytes_string,
        RangeLH_Config::float_scale);
    for (auto &[key, data] : dataset)
        index.insert(key, data);

    for (auto _ : state)
    {
        for (auto &[key, _] : dataset)
            benchmark::DoNotOptimize(index.point_lookup(key));
    }
}
BENCHMARK(BM_RangeLH_PointSearch)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

/* ================= RANGE SEARCH BENCH ================= */
static std::vector<std::pair<uint64_t, uint64_t>>
generate_ranges(size_t Q)
{
    std::vector<std::pair<uint64_t, uint64_t>> ranges;
    ranges.reserve(Q);

    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<uint64_t> dist(1, 1e12);

    const uint64_t range_width = 1e6; // tune this

    for (size_t i = 0; i < Q; i++)
    {
        uint64_t left = dist(rng);
        uint64_t right = left + range_width;
        ranges.emplace_back(left, right);
    }

    return ranges;
}

static void BM_BPlus_RangeQuery(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);
    auto ranges = generate_ranges(10000);

    BPlusTree tree;
    for (auto &[key, data] : dataset)
        tree.insert(key, data);

    for (auto _ : state)
    {
        for (auto &[l, r] : ranges)
            benchmark::DoNotOptimize(tree.rangeSearch(l, r));
    }
}
BENCHMARK(BM_BPlus_RangeQuery)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

static void BM_RangeLH_RangeQuery(benchmark::State &state)
{
    size_t N = state.range(0);
    auto dataset = generate_dataset(N);
    auto ranges = generate_ranges(10000);

    RangeLH index(
        RangeLH_Config::master_init_num_bucket,
        RangeLH_Config::master_split_policy,
        N * SIZEOFFSET, // RangeLH_Config::expected_n_items,
        RangeLH_Config::fp_prob_bloom_RF,
        RangeLH_Config::delta_bloom_RF,
        RangeLH_Config::key_length,
        RangeLH_Config::max_bytes_string,
        RangeLH_Config::float_scale);

    for (auto &[key, data] : dataset)
        index.insert(key, data);

    for (auto _ : state)
    {
        for (auto &[l, r] : ranges)
            benchmark::DoNotOptimize(index.range_lookup(l, r));
    }
}
BENCHMARK(BM_RangeLH_RangeQuery)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE);

/* ================= MEMORY BENCH ================= */

static void BM_BPlus_Memory(benchmark::State &state)
{
    for (auto _ : state)
    {
        size_t N = state.range(0);
        auto dataset = generate_dataset(N);

        BPlusTree tree;
        for (auto &[key, data] : dataset)
            tree.insert(key, data);

        state.counters["Memory"] = getCurrentRSS();

        // Cleanup memory
        for (auto &pair : dataset)
        {
            delete pair.second;
        }
    }
}
BENCHMARK(BM_BPlus_Memory)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE)->Iterations(1);

static void BM_RangeLH_Memory(benchmark::State &state)
{
    for (auto _ : state)
    {
        size_t N = state.range(0);
        auto dataset = generate_dataset(N);

        RangeLH index(
            RangeLH_Config::master_init_num_bucket,
            RangeLH_Config::master_split_policy,
            N * SIZEOFFSET, // RangeLH_Config::expected_n_items,
            RangeLH_Config::fp_prob_bloom_RF,
            RangeLH_Config::delta_bloom_RF,
            RangeLH_Config::key_length,
            RangeLH_Config::max_bytes_string,
            RangeLH_Config::float_scale);
        for (auto &[key, data] : dataset)
            index.insert(key, data);

        state.counters["Memory"] = getCurrentRSS();

        // Cleanup memory
        for (auto &pair : dataset)
        {
            delete pair.second;
        }
    }
}
BENCHMARK(BM_RangeLH_Memory)->RangeMultiplier(10)->Range(MINSIZE, MAXSIZE)->Iterations(1);

BENCHMARK_MAIN();