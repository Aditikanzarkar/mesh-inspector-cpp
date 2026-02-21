#include "parser/STLParser.h"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr std::streamoff kBinaryHeaderBytes = 80;
constexpr std::streamoff kBinaryFacetCountBytes = 4;
constexpr std::streamoff kBinaryFacetBytes = 50;

bool isLikelyBinarySTL(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open STL file: " + filePath);
    }

    file.seekg(0, std::ios::end);
    const std::streamoff fileSize = file.tellg();
    if (fileSize < kBinaryHeaderBytes + kBinaryFacetCountBytes) {
        return false;
    }

    file.seekg(kBinaryHeaderBytes, std::ios::beg);
    std::uint32_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));
    if (!file) {
        return false;
    }

    const std::streamoff expectedSize =
        kBinaryHeaderBytes + kBinaryFacetCountBytes +
        static_cast<std::streamoff>(triangleCount) * kBinaryFacetBytes;
    return expectedSize == fileSize;
}

std::vector<Triangle> parseBinarySTL(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open STL file: " + filePath);
    }

    file.seekg(kBinaryHeaderBytes, std::ios::beg);
    std::uint32_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));
    if (!file) {
        throw std::runtime_error("Failed to read binary STL triangle count.");
    }

    std::vector<Triangle> triangles;
    triangles.reserve(triangleCount);

    for (std::uint32_t i = 0; i < triangleCount; ++i) {
        float normal[3] = {0.0f, 0.0f, 0.0f};
        float v1[3] = {0.0f, 0.0f, 0.0f};
        float v2[3] = {0.0f, 0.0f, 0.0f};
        float v3[3] = {0.0f, 0.0f, 0.0f};
        std::uint16_t attributeByteCount = 0;

        file.read(reinterpret_cast<char*>(normal), sizeof(normal));
        file.read(reinterpret_cast<char*>(v1), sizeof(v1));
        file.read(reinterpret_cast<char*>(v2), sizeof(v2));
        file.read(reinterpret_cast<char*>(v3), sizeof(v3));
        file.read(reinterpret_cast<char*>(&attributeByteCount), sizeof(attributeByteCount));

        if (!file) {
            throw std::runtime_error("Binary STL ended unexpectedly while reading triangles.");
        }

        triangles.push_back(Triangle{
            Vector3{v1[0], v1[1], v1[2]},
            Vector3{v2[0], v2[1], v2[2]},
            Vector3{v3[0], v3[1], v3[2]},
        });
    }

    return triangles;
}

std::vector<Triangle> parseAsciiSTL(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Unable to open STL file: " + filePath);
    }

    std::vector<Triangle> triangles;
    std::vector<Vector3> vertices;
    vertices.reserve(3);

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;

        if (token != "vertex") {
            continue;
        }

        Vector3 v;
        lineStream >> v.x >> v.y >> v.z;
        if (!lineStream) {
            throw std::runtime_error("Invalid vertex line in ASCII STL: " + line);
        }

        vertices.push_back(v);
        if (vertices.size() == 3) {
            triangles.push_back(Triangle{vertices[0], vertices[1], vertices[2]});
            vertices.clear();
        }
    }

    if (!vertices.empty()) {
        throw std::runtime_error("ASCII STL has incomplete triangle vertex data.");
    }

    return triangles;
}
}  // namespace

std::vector<Triangle> STLParser::parseFile(const std::string& filePath) {
    if (isLikelyBinarySTL(filePath)) {
        return parseBinarySTL(filePath);
    }
    return parseAsciiSTL(filePath);
}
