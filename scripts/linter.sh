#!/bin/bash

# Check if clang-format is installed. If not install it
if ! command -v clang-format &> /dev/null; then
    echo "clang-format could not be found. Installing clang-format..."
    sudo apt install clang-format -y
fi

# Check if clang-tidy is installed. If not install it
if ! command -v clang-tidy &> /dev/null; then
    echo "clang-tidy could not be found. Installing clang-tidy..."
    sudo apt install clang-tidy -y
fi

# Get the script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change to project root if not already there
if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

# If build/compile_commands.json does not exist, then run ./compile.sh to generate it
if [ ! -f "build/compile_commands.json" ]; then
    echo "compile_commands.json not found. Running: ./compile.sh to generate it..."
    ./scripts/compile.sh
fi

# Run clang-format and clang-tidy on all cpp and hpp files in the project except grepped directories
for file in $(find . -name '*.cpp' -o -name '*.hpp' | grep -v -e '^./build/' -e '^./tests/' -e '^./.venv/'); do
    echo "Running clang-format on: $file"
    clang-format -i $file

    # If arg format-only skip clang-tidy
    if [ "$1" == "format-only" ]; then
        echo ""
        continue
    fi
    
    echo "Running clang-tidy on: $file"
    clang-tidy -p build \
        -checks='bugprone-*,performance-*,readability-*,portability-*,clang-analyzer-*,cppcoreguidelines-*misc-*' \
        -fix \
        -extra-arg=-std=c++23 \
        -extra-arg=-I$(pwd)/build/include \
        -extra-arg=-I$(pwd)/include \
        $file
    echo ""
done
