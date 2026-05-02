# Numerical Finance Basket Option Pricing Project

Clean C++ project for numerical finance coursework.

The project prices European and Bermudan basket options in a multidimensional Black-Scholes model using Monte Carlo simulation. It includes pseudo-random simulation, Halton quasi-random simulation, static control variates, antithetic variables, and Longstaff-Schwarz pricing for Bermudan options.

## Documentation

- `QUICK_REFERENCE.md`: short command sheet for building, testing, and running the executables.
- `USAGE_EXAMPLES.md`: practical examples showing how to modify products, pricer settings, and report inputs.
- `professor_editable_scenario.cpp`: main file to edit when creating a custom basket option scenario.

## Requirements

- C++ compiler with C++11 support, for example `g++`
- CMake 3.16 or newer
- A build tool available to CMake, for example `make`, `mingw32-make`, or `nmake`

Optional checks:

```bash
cmake --version
g++ --version
```

## Build And Test

From the project root:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

## Main Executables

- `professor_smoke_test`: fixed demo to check that the pricing workflow works.
- `professor_editable_scenario`: main executable for a user-defined product and pricer configuration.
- `report_results`: generates `report_results.csv` locally for report tables and graphs.
- `number_generation_app`: console app for the original random-number-generation part of the project.

On Windows with Git Bash, run for example:

```bash
./build/professor_smoke_test.exe
./build/professor_editable_scenario.exe
```

On Linux/macOS, remove `.exe`:

```bash
./build/professor_smoke_test
./build/professor_editable_scenario
```

## Custom Use

To create a custom option, edit only the clearly marked functions in `professor_editable_scenario.cpp`:

```cpp
BuildProductToModify()
BuildPricerConfigToModify()
```

Then rebuild that executable:

```bash
cmake --build build --target professor_editable_scenario --parallel 2
```

For product examples, variance reduction settings, and report generation, see `USAGE_EXAMPLES.md`.
