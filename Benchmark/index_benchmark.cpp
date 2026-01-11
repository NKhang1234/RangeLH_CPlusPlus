#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include "bplustree_wrapper.h"
#include "linear_hashing_wrapper.h"

using Key = uint64_t;
using Value = uint64_t;

/* ============================
   Dataset Generation
   ============================ */

static std::vector<Key> gen_keys(size_t n) {
    std::mt19937_64 rng(42);
    std::vector<Key> keys(n);
    for (auto& k : keys) k = rng();
    return keys;
}

/* ============================
   Benchmark: Point Lookup
   ============================ */

template<typename IDX>
static void BM_PointLookup(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = gen_keys(N);

    IDX index;
    for (auto k : keys)
        index.insert(k, k);

    Value v;

    for (auto _ : state) {
        benchmark::DoNotOptimize(
            index.find(keys[_ % N], v)
        );
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(BM_PointLookup, BPlusTreeIndex<Key,Value>)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kNanosecond);

BENCHMARK_TEMPLATE(BM_PointLookup, LinearHashingIndex<Key,Value>)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kNanosecond);

/* ============================
   Benchmark: Range Query
   ============================ */

template<typename IDX>
static void BM_RangeQuery(benchmark::State& state) {
    const size_t N = state.range(0);
    auto keys = gen_keys(N);

    IDX index;
    for (auto k : keys)
        index.insert(k, k);

    std::vector<Value> out;
    const Key l = keys[N / 4];
    const Key r = keys[N / 4 + 128];

    for (auto _ : state) {
        out.clear();
        benchmark::DoNotOptimize(
            index.range_query(l, r, out)
        );
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(BM_RangeQuery, BPlusTreeIndex<Key,Value>)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(BM_RangeQuery, LinearHashingIndex<Key,Value>)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
