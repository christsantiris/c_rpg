#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
debug_build="$project_root/build/debug-shortcuts"

cmake -S "$project_root" -B "$debug_build" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$debug_build" --target conr --parallel 4

cd "$project_root"
exec "$debug_build/conr" --unlock-shortcuts "$@"
