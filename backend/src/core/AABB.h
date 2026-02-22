#pragma once


#include <vector>
#include "core/Triangle.h"

struct AABB {
    Vector3 min;
    Vector3 max;
};

AABB triangleBounds(const Triangle& triangle);
bool overlaps(const AABB& a, const AABB& b);
Vector3 boundsExtent(const AABB& bounds);
double boundsVolume(const AABB& bounds);
bool intersectRayAABB(const Vector3& origin, const Vector3& dir, const AABB& box, double epsilon = 1e-12);
AABB computeMeshBounds(const std::vector<Triangle>& triangles);
