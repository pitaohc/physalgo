#pragma once

#include <cstdint>
#include <vector>

#include "core/Body.h"
#include "core/BroadPhase.h"
#include "core/Pair.h"

struct Box3 {
    Vec3 min;
    Vec3 max;

    Vec3 center() const { return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f,
                                  (min.z + max.z) * 0.5f}; }
};

enum class SpawnMode {
    Random,
    AxisX,
    AxisY,
    AxisZ,
    Diagonal,
    Stacked,
};

class World {
public:
    explicit World(const Box3& bounds);

    void setBroadPhase(BroadPhase* broadPhase);
    BroadPhase* broadPhase() const { return broadPhase_; }

    void spawnRandom(int count, std::uint32_t seed);
    void spawnScenario(SpawnMode mode, int count);
    void clear();
    void step(float dt);

    const std::vector<Body>& bodies() const { return bodies_; }
    const std::vector<Pair>& pairs() const { return pairs_; }
    const Box3& bounds() const { return bounds_; }
    double lastBroadPhaseMicros() const { return lastBroadPhaseMicros_; }
    double broadPhaseMicros() const { return broadPhaseMicrosSmooth_; }

private:
    Box3 bounds_;
    std::vector<Body> bodies_;
    std::vector<Pair> pairs_;
    BroadPhase* broadPhase_ = nullptr;
    double lastBroadPhaseMicros_ = 0.0;
    double broadPhaseMicrosSmooth_ = 0.0;
    bool hasMicros_ = false;
};
