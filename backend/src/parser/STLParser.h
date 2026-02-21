#pragma once

#include <string>
#include <vector>

#include "core/Triangle.h"

class STLParser {
public:
    static std::vector<Triangle> parseFile(const std::string& filePath);
};
