#!/usr/bin/env bash
set -euo pipefail

TARGET="${1:-all}"

cmake -S . -B build
cmake --build build -j --target "$TARGET"