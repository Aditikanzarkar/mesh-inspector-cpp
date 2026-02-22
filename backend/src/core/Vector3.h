#pragma once

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

Vector3 add(const Vector3& a, const Vector3& b);
Vector3 subtract(const Vector3& a, const Vector3& b);
Vector3 scale(const Vector3& v, double s);
double dot(const Vector3& a, const Vector3& b);
Vector3 cross(const Vector3& a, const Vector3& b);
double magnitude(const Vector3& v);
Vector3 normalize(const Vector3& v, double epsilon = 1e-12);
double component(const Vector3& v, int axis);
