#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "core/Triangle.h"
#include "geometry/AreaCalculator.h"
#include "parser/STLParser.h"

namespace {
void printVertex(const Vector3& v) {
    std::cout << "(" << v.x << "," << v.y << "," << v.z << ")";
}
}

int main(int argc, char* argv[]) {
    const auto startTime = std::chrono::steady_clock::now();

    if (argc < 2) {
        std::cerr << "Usage: mesh_inspector <path-to-stl-file>\n";
        return 1;
    }

    const std::string filePath = argv[1];

    try {
        const std::vector<Triangle> triangles = STLParser::parseFile(filePath);
        const double area = AreaCalculator::totalArea(triangles);
        const double volume = 0;//VolumeCalculator::estimateVolume(triangles);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "triangles: " << triangles.size() << '\n';
        std::cout << "total area: " << area << '\n';
        std::cout << "estimated volume: " << volume << '\n';
        const auto endTime = std::chrono::steady_clock::now();
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::cout << "runtime: " << elapsedMs << " ms\n";
        // for (std::size_t i = 0; i < triangles.size(); ++i) {
        //     const Triangle& t = triangles[i];
        //     std::cout << "triangle " << (i + 1) << ": ";
        //     printVertex(t.v1);
        //     std::cout << " | ";
        //     printVertex(t.v2);
        //     std::cout << " | ";
        //     printVertex(t.v3);
        //     std::cout << '\n';
        // }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
