#include <cmath>
#include <iostream>
#include <vector>

#include "core/Triangle.h"
#include "geometry/AreaCalculator.h"

namespace {
bool nearlyEqual(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}
}

int main() {
    const Triangle rightTriangle{
        Vector3{0.0, 0.0, 0.0},
        Vector3{1.0, 0.0, 0.0},
        Vector3{0.0, 1.0, 0.0},
    };

    const Triangle shiftedTriangle{
        Vector3{1.0, 1.0, 0.0},
        Vector3{2.0, 1.0, 0.0},
        Vector3{1.0, 2.0, 0.0},
    };

    const double singleArea = AreaCalculator::triangleArea(rightTriangle);
    if (!nearlyEqual(singleArea, 0.5)) {
        std::cerr << "triangleArea failed. expected 0.5 got " << singleArea << '\n';
        return 1;
    }

    const std::vector<Triangle> triangles{rightTriangle, shiftedTriangle};
    const double total = AreaCalculator::totalArea(triangles);
    if (!nearlyEqual(total, 1.0)) {
        std::cerr << "totalArea failed. expected 1.0 got " << total << '\n';
        return 1;
    }

    std::cout << "test_area passed\n";
    return 0;
}
