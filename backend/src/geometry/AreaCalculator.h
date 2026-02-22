#pragma once

#include <vector>

#include "core/Triangle.h"

class AreaCalculator {
public:
    static double triangleArea(const Triangle& triangle);
    static double totalArea(const std::vector<Triangle>& triangles);
};
