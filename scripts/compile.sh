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

# If ./compile.sh release enable release mode
if [ "$1" == "release" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Release
    echo "Release mode enabled"
else
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    echo "Debug mode enabled"
fi

make

# Wait until done compiling and then run tests
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running tests..."
    ctest --output-on-failure 
else
    echo "Compilation failed."
fi
