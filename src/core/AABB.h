#pragma once

#include "Vec3.h"

struct AABB {
    Vec3 min;
    Vec3 max;

    Vec3 center() const {
        return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f,
                (min.z + max.z) * 0.5f};
    }
    Vec3 halfExtents() const {
        return {(max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f,
                (max.z - min.z) * 0.5f};
    }
    Vec3 size() const { return {max.x - min.x, max.y - min.y, max.z - min.z}; }

    static AABB fromCenterHalfExtents(const Vec3& c, const Vec3& h) {
        return {{c.x - h.x, c.y - h.y, c.z - h.z},
                {c.x + h.x, c.y + h.y, c.z + h.z}};
    }

    bool overlaps(const AABB& o) const {
        return min.x <= o.max.x && o.min.x <= max.x &&
               min.y <= o.max.y && o.min.y <= max.y &&
               min.z <= o.max.z && o.min.z <= max.z;
    }

    bool overlapsX(const AABB& o) const {
        return min.x <= o.max.x && o.min.x <= max.x;
    }

    bool overlapsY(const AABB& o) const {
        return min.y <= o.max.y && o.min.y <= max.y;
    }

    bool overlapsZ(const AABB& o) const {
        return min.z <= o.max.z && o.min.z <= max.z;
    }

    bool overlapsYZ(const AABB& o) const {
        return overlapsY(o) && overlapsZ(o);
    }
};
