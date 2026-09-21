#include "core/BruteForce.h"

void BruteForce::computePairs(const std::vector<Body>& bodies,
                              std::vector<Pair>& outPairs) {
    outPairs.clear();

    const int n = static_cast<int>(bodies.size());
    for (int i = 0; i < n; ++i) {
        const AABB a = bodies[i].aabb();
        for (int j = i + 1; j < n; ++j) {
            if (a.overlaps(bodies[j].aabb())) {
                outPairs.push_back(makePair(i, j));
            }
        }
    }
}
