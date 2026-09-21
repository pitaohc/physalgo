#pragma once

#include "BroadPhase.h"

class BruteForce : public BroadPhase {
public:
    const char* name() const override { return "BruteForce"; }

    void computePairs(const std::vector<Body>& bodies,
                      std::vector<Pair>& outPairs) override;
};
