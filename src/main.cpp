#include <benchmark/benchmark.h>

#include "binary_search_tree.hpp"
#include "splay.hpp"
#include "treap.hpp"

static void BinarySearchTreeInsert(benchmark::State& state) {
    int n = state.range(0);
    for (auto _ : state) {
        BinarySearchTree<int> tree;
        for (int i = 0; i < n; ++i) {
            tree.insert(i);
        }
    }
}
BENCHMARK(BinarySearchTreeInsert)->RangeMultiplier(10)->Range(10, 10000);

static void SplayInsert(benchmark::State& state) {
    int n = state.range(0);
    for (auto _ : state) {
        Splay<int> tree;
        for (int i = 0; i < n; ++i) {
            tree.insert(i);
        }
    }
}

BENCHMARK(SplayInsert)->RangeMultiplier(10)->Range(10, 10000);

static void TreapInsert(benchmark::State& state) {
    int n = state.range(0);
    for (auto _ : state) {
        Treap<int> tree;
        for (int i = 0; i < n; ++i) {
            tree.insert(i);
        }
    }
}

BENCHMARK(TreapInsert)->RangeMultiplier(10)->Range(10, 10000);

BENCHMARK_MAIN();