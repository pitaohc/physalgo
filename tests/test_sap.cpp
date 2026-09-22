#include <algorithm>
#include <cstdio>
#include <random>
#include <vector>

#include "core/Body.h"
#include "core/BruteForce.h"
#include "core/Pair.h"
#include "core/SweepAndPrune.h"

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool cond, const char* expr, const char* file, int line) {
    ++g_checks;
    if (!cond) {
        ++g_failures;
        std::printf("FAIL: %s (%s:%d)\n", expr, file, line);
    }
}

#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

std::vector<Pair> sortedUnique(std::vector<Pair> pairs) {
    std::sort(pairs.begin(), pairs.end());
    pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
    return pairs;
}

std::vector<Body> makeBodies(const std::vector<AABB>& boxes) {
    std::vector<Body> bodies;
    bodies.reserve(boxes.size());
    for (const AABB& box : boxes) {
        Body b;
        b.position = box.center();
        b.halfExtents = box.halfExtents();
        bodies.push_back(b);
    }
    return bodies;
}

void compareOnBoxes(const std::vector<AABB>& boxes) {
    const std::vector<Body> bodies = makeBodies(boxes);

    BruteForce brute;
    SweepAndPrune sap;
    std::vector<Pair> brutePairs;
    std::vector<Pair> sapPairs;

    brute.computePairs(bodies, brutePairs);
    sap.computePairs(bodies, sapPairs);

    CHECK(sortedUnique(brutePairs) == sortedUnique(sapPairs));

    for (const Pair& p : sapPairs) {
        CHECK(p.a < p.b);
        CHECK(p.a >= 0 && p.b < static_cast<int>(bodies.size()));
    }
}

void assertPairCount(const std::vector<AABB>& boxes, int expectedPairs) {
    const std::vector<Body> bodies = makeBodies(boxes);

    BruteForce brute;
    SweepAndPrune sap;
    std::vector<Pair> brutePairs;
    std::vector<Pair> sapPairs;

    brute.computePairs(bodies, brutePairs);
    sap.computePairs(bodies, sapPairs);

    const std::vector<Pair> uniqueBrute = sortedUnique(brutePairs);
    const std::vector<Pair> uniqueSap = sortedUnique(sapPairs);

    CHECK(uniqueBrute == uniqueSap);
    CHECK(static_cast<int>(uniqueSap.size()) == expectedPairs);

    for (const Pair& p : sapPairs) {
        CHECK(p.a < p.b);
        CHECK(p.a >= 0 && p.b < static_cast<int>(bodies.size()));
    }
}

std::vector<AABB> randomBoxes(int count, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> pos(-200.0f, 200.0f);
    std::uniform_real_distribution<float> size(4.0f, 40.0f);

    std::vector<AABB> boxes;
    boxes.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const float hx = size(rng) * 0.5f;
        const float hy = size(rng) * 0.5f;
        const float hz = size(rng) * 0.5f;
        const Vec3 c{pos(rng), pos(rng), pos(rng)};
        boxes.push_back(AABB::fromCenterHalfExtents(c, {hx, hy, hz}));
    }
    return boxes;
}

enum class Axis { X, Y, Z };

std::vector<AABB> axisRow(Axis axis, int count, float half, float step) {
    std::vector<AABB> boxes;
    boxes.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) * step;
        Vec3 c{0.0f, 0.0f, 0.0f};
        if (axis == Axis::X) {
            c.x = t;
        } else if (axis == Axis::Y) {
            c.y = t;
        } else {
            c.z = t;
        }
        boxes.push_back(AABB::fromCenterHalfExtents(c, {half, half, half}));
    }
    return boxes;
}

std::vector<AABB> diagonalRow(int count, float half, float step) {
    std::vector<AABB> boxes;
    boxes.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) * step;
        boxes.push_back(AABB::fromCenterHalfExtents({t, t, t}, {half, half, half}));
    }
    return boxes;
}

void testEmptyAndSingle() {
    compareOnBoxes({});
    compareOnBoxes({AABB{{0, 0, 0}, {10, 10, 10}}});
}

void testSeparated() {
    compareOnBoxes({
        AABB{{0, 0, 0}, {10, 10, 10}},
        AABB{{100, 0, 0}, {110, 10, 10}},
        AABB{{0, 100, 0}, {10, 110, 10}},
        AABB{{0, 0, 100}, {10, 10, 110}},
    });
}

void testTouching() {
    compareOnBoxes({
        AABB{{0, 0, 0}, {10, 10, 10}},
        AABB{{10, 0, 0}, {20, 10, 10}},
        AABB{{0, 10, 0}, {10, 20, 10}},
        AABB{{0, 0, 10}, {10, 10, 20}},
        AABB{{10, 10, 10}, {20, 20, 20}},
    });
}

void testNestedAndIdentical() {
    compareOnBoxes({
        AABB{{0, 0, 0}, {100, 100, 100}},
        AABB{{10, 10, 10}, {20, 20, 20}},
        AABB{{30, 30, 30}, {40, 40, 40}},
        AABB{{10, 10, 10}, {20, 20, 20}},
    });
}

void testZSeparation() {
    compareOnBoxes({
        AABB{{0, 0, 0}, {10, 10, 10}},
        AABB{{0, 0, 20}, {10, 10, 30}},
    });
    compareOnBoxes({
        AABB{{0, 0, 0}, {10, 10, 10}},
        AABB{{0, 0, -30}, {10, 10, -20}},
    });
    compareOnBoxes({
        AABB{{0, 0, 0}, {10, 10, 10}},
        AABB{{50, 0, 0}, {60, 10, 10}},
        AABB{{0, 50, 0}, {10, 60, 10}},
    });
}

void testAxisRows() {
    const int n = 8;
    const float h = 5.0f;
    for (Axis axis : {Axis::X, Axis::Y, Axis::Z}) {
        assertPairCount(axisRow(axis, n, h, 2.0f * h), n - 1);
        assertPairCount(axisRow(axis, n, h, 2.0f * h + 1.0f), 0);
        assertPairCount(axisRow(axis, n, h, 2.0f * h - 0.5f), n - 1);
    }
}

void testDiagonal() {
    const int n = 8;
    const float h = 5.0f;
    assertPairCount(diagonalRow(n, h, 2.0f * h), n - 1);
    assertPairCount(diagonalRow(n, h, 2.0f * h + 1.0f), 0);
}

void testStackedAllOverlap() {
    const int n = 8;
    const float h = 5.0f;
    assertPairCount(axisRow(Axis::Y, n, h, 1.0f), n * (n - 1) / 2);
}

void testRandomized() {
    for (std::uint32_t seed = 0; seed < 40; ++seed) {
        compareOnBoxes(randomBoxes(2, seed));
        compareOnBoxes(randomBoxes(16, seed));
        compareOnBoxes(randomBoxes(64, seed));
        compareOnBoxes(randomBoxes(150, seed));
    }
}

void testSapHasNoDuplicates() {
    const std::vector<Body> bodies = makeBodies(randomBoxes(120, 7));
    SweepAndPrune sap;
    std::vector<Pair> pairs;
    sap.computePairs(bodies, pairs);
    CHECK(pairs.size() == sortedUnique(pairs).size());
}

}

int main() {
    testEmptyAndSingle();
    testSeparated();
    testTouching();
    testNestedAndIdentical();
    testZSeparation();
    testAxisRows();
    testDiagonal();
    testStackedAllOverlap();
    testRandomized();
    testSapHasNoDuplicates();

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
