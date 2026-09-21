#include "core/SweepAndPrune.h"

#include <algorithm>

void SweepAndPrune::computePairs(const std::vector<Body>& bodies,
                                 std::vector<Pair>& outPairs) {
    outPairs.clear();
    endpointsX_.clear();
    boxes_.clear();
    active_.clear();

    const std::size_t num_bodies = bodies.size();
    if (num_bodies < 2) {
        return;
    }

    boxes_.reserve(num_bodies);
    for (const Body& body : bodies) {
        boxes_.emplace_back(body.aabb());
    }

    endpointsX_.reserve(num_bodies * 2);
    for (std::size_t i = 0; i < num_bodies; ++i) {
        const AABB& box = boxes_[i];
        endpointsX_.emplace_back(Endpoint{box.min.x, static_cast<int>(i), true});
        endpointsX_.emplace_back(Endpoint{box.max.x, static_cast<int>(i), false});
    }

    std::sort(endpointsX_.begin(),endpointsX_.end(),[](const Endpoint& a, const Endpoint& b) {
        if (a.value != b.value) {return a.value < b.value;}
        return a.isMin && !b.isMin;
    });

    for (const Endpoint& endpoint:endpointsX_) {
        if (endpoint.isMin) {
            const AABB& a = boxes_[endpoint.body];
            for (const int other: active_) {
                if (a.overlapsYZ(boxes_[other])) {
                    outPairs.emplace_back(makePair(endpoint.body,other));
                }
            }
            active_.emplace_back(endpoint.body);
        } else {
            const auto it = std::find(active_.begin(), active_.end(), endpoint.body);
            if (it != active_.end()) {
                active_.erase(it);
            }
        }
    }
}
