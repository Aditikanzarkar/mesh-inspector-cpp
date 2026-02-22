#include "core/AABB.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "core/Vector3.h"

AABB triangleBounds(const Triangle& triangle) {
    AABB box{};
    box.min = Vector3{
        std::min({triangle.v1.x, triangle.v2.x, triangle.v3.x}),
        std::min({triangle.v1.y, triangle.v2.y, triangle.v3.y}),
        std::min({triangle.v1.z, triangle.v2.z, triangle.v3.z}),
    };
    box.max = Vector3{
        std::max({triangle.v1.x, triangle.v2.x, triangle.v3.x}),
        std::max({triangle.v1.y, triangle.v2.y, triangle.v3.y}),
        std::max({triangle.v1.z, triangle.v2.z, triangle.v3.z}),
    };
    return box;
}

bool overlaps(const AABB& a, const AABB& b) {
    return !(a.max.x < b.min.x || a.min.x > b.max.x || a.max.y < b.min.y || a.min.y > b.max.y ||
             a.max.z < b.min.z || a.min.z > b.max.z);
}

Vector3 boundsExtent(const AABB& bounds) {
    return subtract(bounds.max, bounds.min);
}

double boundsVolume(const AABB& bounds) {
    const Vector3 ext = boundsExtent(bounds);
    return std::max(0.0, ext.x) * std::max(0.0, ext.y) * std::max(0.0, ext.z);
}

bool intersectRayAABB(const Vector3& origin, const Vector3& dir, const AABB& box, double epsilon) {
    double tMin = -std::numeric_limits<double>::infinity();
    double tMax = std::numeric_limits<double>::infinity();

    for (int axis = 0; axis < 3; ++axis) {
        const double o = component(origin, axis);
        const double d = component(dir, axis);
        const double bMin = component(box.min, axis);
        const double bMax = component(box.max, axis);

        if (std::fabs(d) <= epsilon) {
            if (o < bMin || o > bMax) {
                return false;
            }
            continue;
        }

        const double invD = 1.0 / d;
        double t0 = (bMin - o) * invD;
        double t1 = (bMax - o) * invD;
        if (t0 > t1) {
            std::swap(t0, t1);
        }

        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);
        if (tMax < tMin) {
            return false;
        }
    }

    return tMax >= std::max(0.0, tMin);
}

AABB computeMeshBounds(const std::vector<Triangle>& triangles) {
    AABB bounds{};
    bounds.min = Vector3{
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
    };
    bounds.max = Vector3{
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(),
    };

    for (const Triangle& triangle : triangles) {
        const AABB tri = triangleBounds(triangle);
        bounds.min.x = std::min(bounds.min.x, tri.min.x);
        bounds.min.y = std::min(bounds.min.y, tri.min.y);
        bounds.min.z = std::min(bounds.min.z, tri.min.z);
        bounds.max.x = std::max(bounds.max.x, tri.max.x);
        bounds.max.y = std::max(bounds.max.y, tri.max.y);
        bounds.max.z = std::max(bounds.max.z, tri.max.z);
    }

    return bounds;
}