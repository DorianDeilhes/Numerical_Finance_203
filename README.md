# Numerical Finance Basket Option Pricing Project (C++)

Clean C++ project for numerical finance coursework.

It provides:
- uniform pseudo-random generators,
- Halton quasi-random numbers,
- discrete distribution generators,
- continuous distribution generators,
- a bivariate normal generator,
- multidimensional Black-Scholes simulation,
- European basket option pricing,
- Bermudan basket option pricing with Longstaff-Schwarz,
- variance reduction methods,
- public professor-facing scenario programs,
- CTest programs to verify behavior.

## Project Structure

```text
.
|- include/
|  |- RandomGenerator.h
|  |- UniformGenerator/
|  |- DiscreteGenerator/
|  |- ContinuousGenerator/
|  |- MonteCarlo/
|  |- Pricing/
|  |- SDE/
|  `- PDE/
|- src/
|  |- RandomGenerator.cpp
|  |- UniformGenerator/
|  |- DiscreteGenerator/
|  |- ContinuousGenerator/
|  |- MonteCarlo/
|  |- Pricing/
|  |- SDE/
|  `- PDE/
|- number_generation_app.cpp
|- professor_smoke_test.cpp
|- professor_editable_scenario.cpp
|- report_results.cpp
|- CMakeLists.txt
`- tests/
```

## Class Hierarchy

- `RandomGenerator` (base class)
  - `UniformGenerator`
    - `PseudoGenerator`
      - `LinearCongruential`
      - `EcuyerCombined`
  - `DiscreteGenerator`
    - `HeadTail`
    - `Bernoulli`
    - `Binomial`
    - `FiniteSet`
    - `Poisson`
  - `ContinuousGenerator`
    - `Exponential`
    - `Normal`
- `BivariateNormal` (uses `Normal` internally)

Pricing modules:
- `EuropeanBasket`
- `BermudanBasket`
- `MonteCarloCore`
- variance reduction helpers for quasi-random, control variate, and antithetic methods

## Prerequisites

- C++ compiler with C++11 support (`g++` or equivalent)
- CMake (version 3.16+)
- A make program available to CMake (`make`, `mingw32-make`, or `nmake`)
- Optional: Git Bash/WSL to run `verify.sh`

### Quick Check Commands

Run these in your shell before building:

```bash
cmake --version
g++ --version
command -v make || command -v mingw32-make || where nmake
```

## Build and Run

### Primary Method: CMake (Recommended)

This project uses CMake as the primary build system.

The CMake architecture compiles all shared source files once into the static library
`numerical_finance`, then links each executable to that library. This keeps the build
lighter than recompiling the full source tree for every test and demo executable.

### Build and Test

From the project root:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

### Run on Windows / Git Bash

```bash
./build/number_generation_app.exe
./build/professor_smoke_test.exe
./build/professor_editable_scenario.exe
./build/report_results.exe
```

### Run on Linux/macOS

```bash
./build/number_generation_app
./build/professor_smoke_test
./build/professor_editable_scenario
./build/report_results
```

### Typical Pricing Workflow

1. Build and test once:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

2. Run the fixed public demo:

```bash
./build/professor_smoke_test.exe
```

3. Create your own product by editing `professor_editable_scenario.cpp`.
Change only the two clearly marked functions:

```cpp
BuildProductToModify()
BuildPricerConfigToModify()
```

Example changes:

- switch `product.exercise_style` between `EuropeanStyle` and `BermudanStyle`,
- change `product.spot_prices`, `product.volatilities`, `product.weights`,
- change `product.strike`, `product.maturity`, `product.risk_free_rate`,
- edit `product.correlation_matrix`,
- edit `product.exercise_dates` for Bermudan products,
- change `config.pricing_method`,
- change `config.random_generator`,
- change `config.path_count`, `config.pair_count`, and `config.pilot_count`.

4. Rebuild only the editable scenario:

```bash
cmake --build build --target professor_editable_scenario --parallel 2
```

5. Run the modified scenario:

```bash
./build/professor_editable_scenario.exe
```

6. Generate report data when needed:

```bash
./build/report_results.exe
```

This writes `report_results.csv` locally.

---

## Implemented Generators

### Uniform
- `LinearCongruential(seed, a, c, m)`
- `EcuyerCombined(seed1, seed2)`
- `HaltonQuasiRandom(dimension, useShift, shiftSeed)`

### Discrete
- `HeadTail(uniformGen)`
- `Bernoulli(p, uniformGen)`
- `Binomial(n, p, uniformGen)`
- `FiniteSet(probas, uniformGen)`
- `Poisson(lambda, algo, uniformGen)` where `algo` is:
  - `FirstAlgorithm`
  - `SecondAlgorithm`

### Continuous
- `Exponential(lambda, algo, uniformGen)` where `algo` is:
  - `InverseDistribution`
  - `ExpoRejectionSampling`
- `Normal(mu, sigma, algo, uniformGen)` where `algo` is:
  - `BoxMuller`
  - `CentralLimitTheorem`
  - `NormalRejectionSampling`
- `BivariateNormal(mu_x, mu_y, sigma_x, sigma_y, rho, algo, uniformGen)`

## Tests

The CMake test suite registers these CTest targets:

- `comprehensive_tests`
- `sde_tests`
- `mc_core_tests`
- `basket_tests`
- `phase3_tests`
- `bermudan_tests`
- `phase6_stress_tests`
- `professor_smoke_test`

Run all tests with:

```bash
ctest --test-dir build --output-on-failure
```

Run one test group with:

```bash
ctest --test-dir build -R basket_tests --output-on-failure
```

## Main Executables

- `number_generation_app`: console app for the original random-number-generation part of the project.
- `professor_smoke_test`: stable public workflow check.
- `professor_editable_scenario`: easiest file for a professor/user to modify.
- `report_results`: generates report-ready CSV output locally.
