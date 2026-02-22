#pragma once

#include <vector>

#include "core/Triangle.h"

class VolumeCalculator {
public:
    static double estimateVolume(const std::vector<Triangle>& triangles);
};
