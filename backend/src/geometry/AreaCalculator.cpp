#include "geometry/AreaCalculator.h"

#include <cmath>

namespace {
Vector3 subtract(const Vector3& a, const Vector3& b) {
    return Vector3{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 cross(const Vector3& a, const Vector3& b) {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

double magnitude(const Vector3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
}  // namespace

double AreaCalculator::triangleArea(const Triangle& triangle) {
    const Vector3 edge1 = subtract(triangle.v2, triangle.v1);
    const Vector3 edge2 = subtract(triangle.v3, triangle.v1);
    const Vector3 crossProduct = cross(edge1, edge2);
    return 0.5 * magnitude(crossProduct);
}

double AreaCalculator::totalArea(const std::vector<Triangle>& triangles) {
    double total = 0.0;
    for (const Triangle& triangle : triangles) {
        total += triangleArea(triangle);
    }
    return total;
}
