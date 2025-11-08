#!/usr/bin/env bash
set -euo pipefail

# --- Configuration ---
CQ_SCRIPT="scripts/find_game_string.cq"
BUILD_DIR="build"
SRC_DIR="src"
OUT_FILE="scripts/find_game_string.txt"

# --- Clean output ---
: > "$OUT_FILE"

# --- Run clang-query recursively on all .cpp files ---
find "$SRC_DIR" -type f -name '*.cpp' | while read -r f; do
    echo "Analyzing $f" | tee -a "$OUT_FILE"
    clang-query \
        -p "$BUILD_DIR" \
        --extra-arg=-DUSING_CLANG_QUERY \
        -f "$CQ_SCRIPT" "$f" \
        | tee -a "$OUT_FILE"
done

echo
echo "✅ Done. Results saved to: $OUT_FILE"
