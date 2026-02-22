#include "core/Vector3.h"

#include <cmath>

Vector3 add(const Vector3& a, const Vector3& b) {
    return Vector3{a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 subtract(const Vector3& a, const Vector3& b) {
    return Vector3{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector3 scale(const Vector3& v, double s) {
    return Vector3{v.x * s, v.y * s, v.z * s};
}

double dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 cross(const Vector3& a, const Vector3& b) {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

double magnitude(const Vector3& v) {
    return std::sqrt(dot(v, v));
}

Vector3 normalize(const Vector3& v, double epsilon) {
    const double len = magnitude(v);
    if (len <= epsilon) {
        return Vector3{1.0, 0.0, 0.0};
    }
    return scale(v, 1.0 / len);
}

double component(const Vector3& v, int axis) {
    if (axis == 0) {
        return v.x;
    }
    if (axis == 1) {
        return v.y;
    }
    return v.z;
}
