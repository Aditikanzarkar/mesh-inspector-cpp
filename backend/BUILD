package(default_visibility = ["//visibility:public"])

COPTS = [
    "-std=c++17",
    "-O0",
    "-g",
    "-finstrument-functions",
    "-finstrument-functions-exclude-file-list=/usr/include,/usr/lib",
    "-fno-omit-frame-pointer",
]

cc_library(
    name = "core",
    srcs = [
        "src/core/AABB.cpp",
        "src/core/Vector3.cpp",
    ],
    hdrs = [
        "src/core/AABB.h",
        "src/core/Triangle.h",
        "src/core/Vector3.h",
    ],
    includes = ["src"],
    copts = COPTS,
)

cc_library(
    name = "geometry",
    srcs = [
        "src/geometry/AreaCalculator.cpp",
        "src/geometry/VolumeCalculator.cpp",
    ],
    hdrs = [
        "src/geometry/AreaCalculator.h",
        "src/geometry/VolumeCalculator.h",
    ],
    includes = ["src"],
    copts = COPTS,
    deps = [":core"],
)

cc_library(
    name = "parser",
    srcs = ["src/parser/STLParser.cpp"],
    hdrs = ["src/parser/STLParser.h"],
    includes = ["src"],
    copts = COPTS,
    deps = [":core"],
)

cc_binary(
    name = "mesh_inspector",
    srcs = ["src/main.cpp"],
    deps = [
        ":core",
        ":geometry",
        ":parser",
    ],
    copts = COPTS,
    linkopts = [
        "-Wl,--export-dynamic",
        "-ldl",
    ],
)

cc_binary(
    name = "stl_generator",
    srcs = ["tests/stl_generator.cpp"],
    copts = COPTS,
)

cc_test(
    name = "test_area",
    srcs = ["tests/test_area.cpp"],
    deps = [
        ":core",
        ":geometry",
    ],
    copts = COPTS,
)

cc_test(
    name = "test_volume",
    srcs = ["tests/test_volume.cpp"],
    deps = [
        ":core",
        ":geometry",
    ],
    copts = COPTS,
)
