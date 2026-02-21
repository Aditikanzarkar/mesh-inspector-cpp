add_executable(mesh_inspector
    src/main.cpp
    src/parser/STLParser.cpp
)

target_include_directories(mesh_inspector PRIVATE src)
