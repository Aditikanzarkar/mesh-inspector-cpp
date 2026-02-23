#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
TARGET="${TARGET:-mesh_inspector}"

if [[ $# -gt 0 ]]; then
  case "$1" in
    -h|--help)
      cat <<'EOF'
Usage:
  ./scripts/run.sh [target] [--] [args...]

Examples:
  ./scripts/run.sh mesh_inspector sphere.stl
  ./scripts/run.sh test_area
  ./scripts/run.sh -- sphere.stl

Notes:
  - If first argument matches a known CMake target, that target is run.
  - Otherwise, default target is used and all args are forwarded.
EOF
      exit 0
      ;;
    --)
      shift
      ;;
  esac
fi

if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
  cmake -S . -B "$BUILD_DIR"
fi

if [[ $# -gt 0 ]]; then
  CMAKE_TARGETS="$(cmake --build "$BUILD_DIR" --target help | awk '/^\.\.\. / {print $2}')"
  if printf '%s\n' "$CMAKE_TARGETS" | grep -Fxq "$1"; then
    TARGET="$1"
    shift
  fi
fi

cmake --build "$BUILD_DIR" -j --target "$TARGET"

BIN_PATH="$BUILD_DIR/backend/$TARGET"
if [[ ! -x "$BIN_PATH" ]]; then
  # Fallback for generators/layouts that place binaries elsewhere in the build tree.
  BIN_PATH="$(find "$BUILD_DIR" -type f -name "$TARGET" -perm -u+x | head -n1 || true)"
fi

if [[ -z "${BIN_PATH:-}" || ! -x "$BIN_PATH" ]]; then
  echo "error: built target '$TARGET' but could not locate executable in '$BUILD_DIR'" >&2
  exit 1
fi

exec "$BIN_PATH" "$@"
