[![Build and Test](https://github.com/P6-CMMR/palloc/actions/workflows/build_and_test.yml/badge.svg?branch=main)](https://github.com/P6-CMMR/palloc/actions/workflows/build_and_test.yml)
[![Linting](https://github.com/P6-CMMR/palloc/actions/workflows/linting.yml/badge.svg?branch=main)](https://github.com/P6-CMMR/palloc/actions/workflows/linting.yml)

# Palloc
This repository contains the implementation of the palloc simulator, along with preprocessing and analysis tools used to conduct experiments for a bachelor's thesis focused on parking assignment for autonomous vehicles.

## Environment Files
Environment files for the four cities can be found in the ```environment/``` folder.

To generate environment files from scratch Ubuntu run:
```bash
./scripts/setup.sh
```
## Run Palloc Solver
To run the palloc solver download the executable under the latest release for your platform. You can then run it from the command line with the default settings by inputting an enviroment file with the ```-e <file-path>``` flag. Further options can be seen with the ```-h``` flag.

### Advanced Statistics
To get more advanced statistics of a single or even multiple configurations you can clone the repository and use the python scripts in the ```analysis/``` folder and creating a virtual environment with the packages in ````requirements.txt``` installed.

#### Experiments
To run experiments use
```bash
python3 analysis/run_experiments [options]
```

#### Generating Reports Manually
From project directory run:
```bash
python3 analysis/generate_report.py <env-file-name> <output-file-name>
```
or to generate a report of multiple simulations:
```bash
python3 analysis/generate_report.py <env-file-name> <output-folder>
```

###  How to Compile From Scratch
To compile the executable from scratch ensure you have the following dependencies_
- `cmake >= 3.28`
- `c++23 compiler` (specifically tested with g++-14.2)

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