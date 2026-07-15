#pragma once

#include <string>
#include <vector>

#include "core/Triangle.h"

class STLParser {
public:
    static std::vector<Triangle> parseFile(const std::string& filePath);
    static std::vector<Triangle> parseFromMemory(const std::string& fileData, const std::string& fileName);
};
