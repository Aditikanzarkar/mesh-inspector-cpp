add_executable(mesh_inspector
    src/main.cpp
    src/core/Vector3.cpp
    src/core/AABB.cpp
    src/geometry/AreaCalculator.cpp
    src/geometry/VolumeCalculator.cpp
    src/parser/STLParser.cpp
)

target_include_directories(mesh_inspector PRIVATE src)

add_executable(stl_generator
    tests/stl_generator.cpp
)

target_include_directories(stl_generator PRIVATE tests)

add_executable(test_area
    tests/test_area.cpp
    src/core/Vector3.cpp
    src/geometry/AreaCalculator.cpp
)

target_include_directories(test_area PRIVATE src)

add_executable(test_volume
    tests/test_volume.cpp
    src/core/Vector3.cpp
    src/core/AABB.cpp
    src/geometry/VolumeCalculator.cpp
)

target_include_directories(test_volume PRIVATE src)
