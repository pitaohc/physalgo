#pragma once

#include <vector>

#include "BroadPhase.h"

class SweepAndPrune : public BroadPhase {
public:
    struct Endpoint {
        float value = 0.0f;
        int body = -1;
        bool isMin = true;
    };

    const char* name() const override { return "SweepAndPrune"; }

    void computePairs(const std::vector<Body>& bodies,
                      std::vector<Pair>& outPairs) override;

    const std::vector<Endpoint>& xEndpoints() const { return endpointsX_; }
    const std::vector<int>& activeSet() const { return active_; }

private:
    std::vector<Endpoint> endpointsX_;
    std::vector<AABB> boxes_;
    std::vector<int> active_;
};
