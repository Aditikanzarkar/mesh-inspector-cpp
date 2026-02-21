add_executable(mesh_inspector
    src/main.cpp
    src/parser/STLParser.cpp
)

target_include_directories(mesh_inspector PRIVATE src)

add_executable(stl_generator
    tests/stl_generator.cpp
)

target_include_directories(stl_generator PRIVATE tests)
