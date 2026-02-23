# mesh-inspector-cpp
C++ tool to find info about stl file. 3js for frotend. C++ for backend.

## Quick run (Bazel-like)
Run build+execute in one command (rebuilds only when needed):

```bash
./scripts/run.sh [target] [args...]
```

Optional env vars:
- `TARGET` (default: `mesh_inspector`)
- `BUILD_DIR` (default: `build`)

Example:

```bash
./scripts/run.sh mesh_inspector sphere.stl
./scripts/run.sh test_area
./scripts/run.sh -- sphere.stl
```

https://chatgpt.com/share/699200dd-5fb8-8013-8c03-7787fd8fdfe4
