#pragma once

#include <vector>

#include "Body.h"
#include "Pair.h"

class BroadPhase {
public:
    virtual ~BroadPhase() = default;

    virtual const char* name() const = 0;

    virtual void computePairs(const std::vector<Body>& bodies,
                              std::vector<Pair>& outPairs) = 0;
};
