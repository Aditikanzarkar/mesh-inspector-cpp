#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser/STLParser.h"

int main() {
    const std::string asciiStl =
        "solid demo\n"
        "facet normal 0 0 1\n"
        "  outer loop\n"
        "    vertex 0 0 0\n"
        "    vertex 1 0 0\n"
        "    vertex 0 1 0\n"
        "  endloop\n"
        "endfacet\n"
        "endsolid demo\n";

    const std::vector<Triangle> triangles = STLParser::parseFromMemory(asciiStl, "demo.stl");
    assert(triangles.size() == 1);

    return 0;
}
