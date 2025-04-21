#!/bin/bash
# Get the script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change to project root if not already there
if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

# If ./compile.sh clean
if [ "$1" == "clean" ]; then
    rm -rf build
fi

# Check if build directory exists
if [ ! -d build ]; then
    mkdir build
fi

cd build

case "$1" in
    "release")
        cmake -DCMAKE_BUILD_TYPE=Release .. 
        echo "Release mode enabled"
        ;;
    "perf")
        cmake -DCMAKE_BUILD_TYPE=Perf .. 
        echo "Performance profiling mode enabled"
        ;;
    *)
        cmake -DCMAKE_BUILD_TYPE=Debug .. 
        echo "Debug mode enabled"
        ;;
esac

make

# Wait until done compiling and then run tests
if [ $? -eq 0 ] && [ "$1" != "release" ] && [ "$1" != "perf" ]; then
    echo "Compilation successful. Running tests..."
    ctest --output-on-failure 
else
    echo "Compilation successful."
fi
