#!/bin/bash
# Visual regression test for Moui
# Usage: ./test/visual_test.sh [update]
#   No args: compare against baseline
#   "update": regenerate baseline

set -e
cd "$(dirname "$0")/.."

BASELINE=test/screenshots/baseline
CURRENT=test/screenshots/current
SIM=build/host/simulator/moui_sim

mkdir -p "$BASELINE" "$CURRENT"

if [ ! -f "$SIM" ]; then
    echo "Error: $SIM not found. Run cmake --build build first."
    exit 1
fi

echo "Generating screenshots..."
"$SIM" --screenshot "$CURRENT"

if [ "$1" = "update" ]; then
    cp "$CURRENT"/*.pbm "$BASELINE/"
    echo "Baseline updated: $(ls "$BASELINE"/*.pbm | wc -l) screenshots"
    exit 0
fi

if [ ! "$(ls -A "$BASELINE" 2>/dev/null)" ]; then
    echo "No baseline found. Run: $0 update"
    exit 1
fi

echo ""
echo "Comparing against baseline..."
PASS=0
FAIL=0
MISSING=0

for base in "$BASELINE"/*.pbm; do
    name=$(basename "$base")
    curr="$CURRENT/$name"

    if [ ! -f "$curr" ]; then
        echo "  MISSING: $name"
        MISSING=$((MISSING + 1))
        continue
    fi

    if diff -q "$base" "$curr" > /dev/null 2>&1; then
        printf "  %-30s PASS\n" "$name"
        PASS=$((PASS + 1))
    else
        printf "  %-30s FAIL (pixels differ)\n" "$name"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "$PASS passed, $FAIL failed, $MISSING missing"

if [ $FAIL -gt 0 ] || [ $MISSING -gt 0 ]; then
    echo "Visual regression detected!"
    exit 1
fi

echo "All screenshots match baseline."
exit 0
