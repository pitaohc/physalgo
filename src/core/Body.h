#pragma once

#include "AABB.h"

struct Body {
    Vec3 position;
    Vec3 velocity;
    Vec3 halfExtents{8.0f, 8.0f, 8.0f};
    bool active = true;

    AABB aabb() const {
        return AABB::fromCenterHalfExtents(position, halfExtents);
    }
};
