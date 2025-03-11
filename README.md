[![Build and Test](https://github.com/P6-CMMR/palloc/actions/workflows/build_and_test.yml/badge.svg?branch=main)](https://github.com/P6-CMMR/palloc/actions/workflows/build_and_test.yml)
[![Linting](https://github.com/P6-CMMR/palloc/actions/workflows/linting.yml/badge.svg?branch=main)](https://github.com/P6-CMMR/palloc/actions/workflows/linting.yml)

# Palloc
## Setup & Preprocessing
To setup and preprocess the data on Ubuntu run:
```bash
./scripts/setup.sh
```

## How to Compile
### Dependencies
- `cmake >= 3.28`
- `c++23 compiler`

For Ubuntu simply run:
```bash
./scripts/compile.sh
```

On other platforms:
```bash
mkdir build
cd build
cmake ..; cmake --build .
```
