[![Build and Test](https://github.com/P6-CMMR/palloc/actions/workflows/build_and_test.yaml/badge.svg?branch=main)](https://github.com/P6-CMMR/palloc/actions/workflows/build_and_test.yaml)
[![Linting](https://github.com/P6-CMMR/palloc/actions/workflows/linting.yaml/badge.svg?branch=main)](https://github.com/P6-CMMR/palloc/actions/workflows/linting.yaml)

# PALLOC
## How to Compile
### Dependencies
- `cmake >= 3.28`
- `c++20 compiler`

For Ubuntu simply run:
```bash
./scripts/compile.sh
```

On other platforms:
```
mkdir build
cd build
cmake ..; cmake --build .
```
