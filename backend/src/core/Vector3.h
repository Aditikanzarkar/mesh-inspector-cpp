#pragma once

#include <cmath>

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    inline Vector3 operator+(const Vector3& other) const {
        return Vector3{x + other.x, y + other.y, z + other.z};
    }

    inline Vector3 operator-(const Vector3& other) const {
        return Vector3{x - other.x, y - other.y, z - other.z};
    }

    inline Vector3 operator*(double scalar) const {
        return Vector3{x * scalar, y * scalar, z * scalar};
    }

    inline Vector3 operator/(double scalar) const {
        return Vector3{x / scalar, y / scalar, z / scalar};
    }

    inline Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    inline Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    inline Vector3& operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    inline Vector3& operator/=(double scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }
};

inline Vector3 operator*(double scalar, const Vector3& v) {
    return v * scalar;
}

inline Vector3 add(const Vector3& a, const Vector3& b) {
    return a + b;
}

inline Vector3 subtract(const Vector3& a, const Vector3& b) {
    return a - b;
}

inline Vector3 scale(const Vector3& v, double s) {
    return v * s;
}

inline double dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 cross(const Vector3& a, const Vector3& b) {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

inline double magnitude(const Vector3& v) {
    return std::sqrt(dot(v, v));
}

inline Vector3 normalize(const Vector3& v, double epsilon = 1e-12) {
    const double len = magnitude(v);
    if (len <= epsilon) {
        return Vector3{1.0, 0.0, 0.0};
    }
    return v / len;
}

inline double component(const Vector3& v, int axis) {
    if (axis == 0) {
        return v.x;
    }
    if (axis == 1) {
        return v.y;
    }
    return v.z;
}
