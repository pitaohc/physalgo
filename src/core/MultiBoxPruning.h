//
// Created by can.hu on 2026/9/22.
//
#pragma once


#include "BroadPhase.h"


class MultiBoxPruning : public BroadPhase {
public:
    const char *name() const override { return "MultiBoxPruning"; }

    void computePairs(const std::vector<Body> &bodies, std::vector<Pair> &outPairs) override;
};
