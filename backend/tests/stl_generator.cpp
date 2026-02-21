#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <cstring>

constexpr double PI = 3.14159265358979323846;

struct Vec3 {
    float x, y, z;
};

Vec3 normalize(const Vec3& v) {
    float len = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return { v.x/len, v.y/len, v.z/len };
}

Vec3 computeNormal(const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 u{ b.x - a.x, b.y - a.y, b.z - a.z };
    Vec3 v{ c.x - a.x, c.y - a.y, c.z - a.z };

    Vec3 n{
        u.y*v.z - u.z*v.y,
        u.z*v.x - u.x*v.z,
        u.x*v.y - u.y*v.x
    };
    return normalize(n);
}

Vec3 spherePoint(double radius, double theta, double phi) {
    return {
        static_cast<float>(radius * std::sin(theta) * std::cos(phi)),
        static_cast<float>(radius * std::sin(theta) * std::sin(phi)),
        static_cast<float>(radius * std::cos(theta))
    };
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage: sphere <triangle_count> <output_file>\n";
        return 1;
    }

    uint64_t requestedTriangles = std::stoull(argv[1]);
    const char* filename = argv[2];

    double radius = 50.0;  // 100m diameter

    // Estimate grid resolution
    uint64_t steps = static_cast<uint64_t>(std::sqrt(requestedTriangles / 2.0));
    uint64_t latSteps = steps;
    uint64_t lonSteps = steps;

    uint64_t actualTriangles = 2ULL * latSteps * lonSteps;

    std::cout << "Generating sphere:\n";
    std::cout << "Lat steps: " << latSteps << "\n";
    std::cout << "Lon steps: " << lonSteps << "\n";
    std::cout << "Actual triangles: " << actualTriangles << "\n";

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open file\n";
        return 1;
    }

    // Write 80-byte header
    char header[80] = {};
    std::strncpy(header, "Generated Sphere STL", 80);
    out.write(header, 80);

    // Write triangle count (uint32_t per STL spec)
    uint32_t triCount32 = static_cast<uint32_t>(actualTriangles);
    out.write(reinterpret_cast<char*>(&triCount32), 4);

    double dTheta = PI / latSteps;
    double dPhi   = 2.0 * PI / lonSteps;

    for (uint64_t i = 0; i < latSteps; ++i) {
        double theta1 = i * dTheta;
        double theta2 = (i + 1) * dTheta;

        for (uint64_t j = 0; j < lonSteps; ++j) {
            double phi1 = j * dPhi;
            double phi2 = (j + 1) * dPhi;

            Vec3 p1 = spherePoint(radius, theta1, phi1);
            Vec3 p2 = spherePoint(radius, theta2, phi1);
            Vec3 p3 = spherePoint(radius, theta2, phi2);
            Vec3 p4 = spherePoint(radius, theta1, phi2);

            // Triangle 1
            {
                Vec3 normal = computeNormal(p1, p2, p3);
                out.write(reinterpret_cast<char*>(&normal), 12);
                out.write(reinterpret_cast<char*>(&p1), 12);
                out.write(reinterpret_cast<char*>(&p2), 12);
                out.write(reinterpret_cast<char*>(&p3), 12);
                uint16_t attr = 0;
                out.write(reinterpret_cast<char*>(&attr), 2);
            }

            // Triangle 2
            {
                Vec3 normal = computeNormal(p1, p3, p4);
                out.write(reinterpret_cast<char*>(&normal), 12);
                out.write(reinterpret_cast<char*>(&p1), 12);
                out.write(reinterpret_cast<char*>(&p3), 12);
                out.write(reinterpret_cast<char*>(&p4), 12);
                uint16_t attr = 0;
                out.write(reinterpret_cast<char*>(&attr), 2);
            }
        }
    }

    out.close();
    std::cout << "Done.\n";
    return 0;
}