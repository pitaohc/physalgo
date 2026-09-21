#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

#include "core/Body.h"
#include "core/BruteForce.h"
#include "core/SweepAndPrune.h"

namespace {

std::vector<Body> makeRandomBodies(int count, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> pos(-500.0f, 500.0f);
    std::uniform_real_distribution<float> half(4.0f, 20.0f);

    std::vector<Body> bodies;
    bodies.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        Body b;
        b.halfExtents = {half(rng), half(rng), half(rng)};
        b.position = {pos(rng), pos(rng), pos(rng)};
        bodies.push_back(b);
    }
    return bodies;
}

template <typename Fn>
double timeMicros(Fn&& fn, int iterations) {
    const auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        fn();
    }
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::micro>(end - start).count() /
           static_cast<double>(iterations);
}

}

int main() {
    const int counts[] = {100, 250, 500, 1000, 2000};
    const int iterations = 50;

    std::printf("%8s %12s %12s %12s %10s\n",
                "bodies", "bruteforce", "sap", "speedup", "pairs");

    for (int count : counts) {
        const std::vector<Body> bodies = makeRandomBodies(count, 42);

        BruteForce brute;
        SweepAndPrune sap;
        std::vector<Pair> pairs;

        const double bruteMicros = timeMicros(
            [&] { brute.computePairs(bodies, pairs); }, iterations);
        const double sapMicros = timeMicros(
            [&] { sap.computePairs(bodies, pairs); }, iterations);

        std::printf("%8d %10.1fus %10.1fus %10.2fx %10zu\n",
                    count, bruteMicros, sapMicros,
                    bruteMicros / sapMicros, pairs.size());
    }

    return 0;
}
