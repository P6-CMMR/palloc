#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

PERF_EXE="${PROJECT_ROOT}/tools/perf_wsl2"
FLAMEGRAPH_DIR="${PROJECT_ROOT}/tools/FlameGraph"

if [ ! -x "$PERF_EXE" ]; then
    echo "Error: $PERF_EXE not found or not executable"
    exit 1
fi

if [ ! -d "$FLAMEGRAPH_DIR" ]; then
    echo "Error: FlameGraph tools not found in ${FLAMEGRAPH_DIR}"
    exit 1
fi

mkdir -p perf

./scripts/compile.sh perf

"$PERF_EXE" record --call-graph dwarf,8192 \
    -e cycles:u \
    --strict-freq \
    ./build/palloc-linux/bin/palloc -e data.json

"$PERF_EXE" script -i perf.data | \
    ${FLAMEGRAPH_DIR}/stackcollapse-perf.pl | \
    ${FLAMEGRAPH_DIR}/flamegraph.pl \
    --colors java \
    --hash \
    --width 1900 \
    --title "Palloc CPU Flame Graph (Application Only)" \
    --countname "samples" > perf/flamegraph.html

mv perf.data perf/
echo "Flamegraph generated at: ${PROJECT_ROOT}/perf/flamegraph.html"

"$PERF_EXE" report -i perf/perf.data
