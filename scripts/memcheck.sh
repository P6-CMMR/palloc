#!/bin/bash
# Get the script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change to project root if not already there
if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

# Check if valgrind is installed and install if not
if ! command -v valgrind &> /dev/null; then
    echo "valgrind could not be found. Installing valgrind..."
    sudo apt install valgrind -y
fi

# Check for memory leaks
echo "Checking for memory leaks..."

if [ -z "$1" ]; then
    echo "Usage: $0 <input_file>"
    exit
fi

cat > ortools.supp << EOF
{
   OR-Tools-Protobuf-Static-Memory
   Memcheck:Leak
   match-leak-kinds: possible
   fun:_Znwm
   ...
   obj:*libprotobuf.so*
}
{
   OR-Tools-Absl-Static-Memory
   Memcheck:Leak
   match-leak-kinds: possible
   fun:_Znwm
   ...
   obj:*libabsl_*.so*
}
{
   OR-Tools-Static-Memory
   Memcheck:Leak
   match-leak-kinds: possible
   fun:_Znwm
   ...
   obj:*libortools.so*
}
EOF

# Run valgrind with the suppression file
valgrind --leak-check=full \
         --errors-for-leak-kinds=definite \
         --error-exitcode=1 \
         --suppressions=./ortools.supp \
         ./build/palloc -e "$1"

# Remove the temporary suppression file
rm ortools.supp