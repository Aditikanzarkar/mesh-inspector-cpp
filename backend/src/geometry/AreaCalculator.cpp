#include "geometry/AreaCalculator.h"

double AreaCalculator::triangleArea(const Triangle& triangle) {
    const Vector3 edge1 = triangle.v2 - triangle.v1;
    const Vector3 edge2 = triangle.v3 - triangle.v1;
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
