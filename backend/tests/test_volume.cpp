#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "core/Triangle.h"
#include "geometry/VolumeCalculator.h"
#include "parser/STLParser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: test_volume <path-to-sphere_test.stl>\n";
        return 1;
    }

    const std::string filePath = argv[1];
    const std::vector<Triangle> triangles = STLParser::parseFile(filePath);
    const double estimated = VolumeCalculator::estimateVolume(triangles);

    std::cout << "estimateVolume(" << filePath << ") = " << estimated << '\n';
    return 0;
}
