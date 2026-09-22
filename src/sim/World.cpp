#include "sim/World.h"

#include <chrono>
#include <random>

World::World(const Box3& bounds) : bounds_(bounds) {}

void World::setBroadPhase(BroadPhase* broadPhase) {
    broadPhase_ = broadPhase;
    hasMicros_ = false;
    broadPhaseMicrosSmooth_ = 0.0;
}

void World::clear() {
    bodies_.clear();
    pairs_.clear();
    hasMicros_ = false;
    broadPhaseMicrosSmooth_ = 0.0;
}

void World::spawnRandom(int count, std::uint32_t seed) {
    bodies_.clear();
    pairs_.clear();
    hasMicros_ = false;
    broadPhaseMicrosSmooth_ = 0.0;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> speed(-90.0f, 90.0f);
    std::uniform_real_distribution<float> half(6.0f, 20.0f);

    const float margin = 24.0f;
    for (int i = 0; i < count; ++i) {
        Body b;
        b.halfExtents = {half(rng), half(rng), half(rng)};

        std::uniform_real_distribution<float> px(
            bounds_.min.x + margin + b.halfExtents.x,
            bounds_.max.x - margin - b.halfExtents.x);
        std::uniform_real_distribution<float> py(
            bounds_.min.y + margin + b.halfExtents.y,
            bounds_.max.y - margin - b.halfExtents.y);
        std::uniform_real_distribution<float> pz(
            bounds_.min.z + margin + b.halfExtents.z,
            bounds_.max.z - margin - b.halfExtents.z);

        b.position = {px(rng), py(rng), pz(rng)};
        b.velocity = {speed(rng), speed(rng), speed(rng)};
        bodies_.push_back(b);
    }
}

void World::spawnScenario(SpawnMode mode, int count) {
    bodies_.clear();
    pairs_.clear();
    hasMicros_ = false;
    broadPhaseMicrosSmooth_ = 0.0;

    const float half = 20.0f;
    const Vec3 center = bounds_.center();
    const int n = count > 0 ? count : 1;

    auto makeRow = [&](const Vec3& dir, float step) {
        const float span = static_cast<float>(n - 1) * step;
        for (int i = 0; i < n; ++i) {
            const float t = -span * 0.5f + static_cast<float>(i) * step;
            Body b;
            b.halfExtents = {half, half, half};
            b.position = center + dir * t;
            b.velocity = {0.0f, 0.0f, 0.0f};
            bodies_.push_back(b);
        }
    };

    switch (mode) {
        case SpawnMode::Random:
            spawnRandom(n, 2024);
            return;
        case SpawnMode::AxisX:
            makeRow({1.0f, 0.0f, 0.0f}, half * 2.0f);
            break;
        case SpawnMode::AxisY:
            makeRow({0.0f, 1.0f, 0.0f}, half * 2.0f);
            break;
        case SpawnMode::AxisZ:
            makeRow({0.0f, 0.0f, 1.0f}, half * 2.0f);
            break;
        case SpawnMode::Diagonal:
            makeRow({1.0f, 1.0f, 1.0f}, half * 2.0f);
            break;
        case SpawnMode::Stacked:
            makeRow({0.0f, 1.0f, 0.0f}, 3.0f);
            break;
    }
}

void World::step(float dt) {
    for (Body& b : bodies_) {
        b.position += b.velocity * dt;

        const AABB box = b.aabb();
        if (box.min.x < bounds_.min.x) {
            b.position.x = bounds_.min.x + b.halfExtents.x;
            b.velocity.x = -b.velocity.x;
        } else if (box.max.x > bounds_.max.x) {
            b.position.x = bounds_.max.x - b.halfExtents.x;
            b.velocity.x = -b.velocity.x;
        }

        if (box.min.y < bounds_.min.y) {
            b.position.y = bounds_.min.y + b.halfExtents.y;
            b.velocity.y = -b.velocity.y;
        } else if (box.max.y > bounds_.max.y) {
            b.position.y = bounds_.max.y - b.halfExtents.y;
            b.velocity.y = -b.velocity.y;
        }

        if (box.min.z < bounds_.min.z) {
            b.position.z = bounds_.min.z + b.halfExtents.z;
            b.velocity.z = -b.velocity.z;
        } else if (box.max.z > bounds_.max.z) {
            b.position.z = bounds_.max.z - b.halfExtents.z;
            b.velocity.z = -b.velocity.z;
        }
    }

    if (broadPhase_ != nullptr) {
        const auto start = std::chrono::high_resolution_clock::now();
        broadPhase_->computePairs(bodies_, pairs_);
        const auto end = std::chrono::high_resolution_clock::now();
        lastBroadPhaseMicros_ =
            std::chrono::duration<double, std::micro>(end - start).count();

        constexpr double alpha = 0.05;
        broadPhaseMicrosSmooth_ = hasMicros_
            ? broadPhaseMicrosSmooth_ * (1.0 - alpha) + lastBroadPhaseMicros_ * alpha
            : lastBroadPhaseMicros_;
        hasMicros_ = true;
    }
}
