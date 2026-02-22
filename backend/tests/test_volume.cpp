#include <cmath>
#include <iostream>
#include <vector>

#include "core/Triangle.h"
#include "geometry/VolumeCalculator.h"

namespace {
bool nearlyEqual(double a, double b, double eps) {
    return std::fabs(a - b) <= eps;
}
}

int main() {
    // Tetrahedron with vertices:
    // A(0,0,0), B(1,0,0), C(0,1,0), D(0,0,1) has exact volume 1/6.
    const Vector3 a{0.0, 0.0, 0.0};
    const Vector3 b{1.0, 0.0, 0.0};
    const Vector3 c{0.0, 1.0, 0.0};
    const Vector3 d{0.0, 0.0, 1.0};

    const std::vector<Triangle> tetra{
        Triangle{a, c, b},
        Triangle{a, b, d},
        Triangle{a, d, c},
        Triangle{b, c, d},
    };

    const double estimated = VolumeCalculator::estimateVolume(tetra);
    const double expected = 1.0 / 6.0;
    if (!nearlyEqual(estimated, expected, 0.03)) {
        std::cerr << "estimateVolume(tetra) failed. expected ~" << expected << " got " << estimated << '\n';
        return 1;
    }

    const std::vector<Triangle> empty;
    if (!nearlyEqual(VolumeCalculator::estimateVolume(empty), 0.0, 1e-12)) {
        std::cerr << "estimateVolume(empty) failed. expected 0.0\n";
        return 1;
    }

    std::cout << "test_volume passed\n";
    return 0;
}
