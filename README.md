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

### Windows (Git Bash + g++)

Use this exact sequence from the project root:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Run the number-generation app:

```bash
./build/number_generation_app.exe
```

Run professor-facing examples:

```bash
./build/professor_smoke_test.exe
./build/professor_editable_scenario.exe
./build/report_results.exe
```

### Linux/macOS

From the project root:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Run the number-generation app:

```bash
./build/number_generation_app
```

Run professor-facing examples:

```bash
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

You do not edit `.exe` files directly. You edit the `.cpp` source file, rebuild, then run the new executable.

### Build Parallelism Note

The command `cmake --build build --parallel 2` limits compilation to 2 parallel jobs.
This is safer on laptops with limited RAM.

Avoid using plain `-j` without a number, because it can use too many jobs and consume several GB of RAM.
If the computer is still low on memory, use the fully sequential command:

```bash
cmake --build build
```

To build only one executable, use:

```bash
cmake --build build --target professor_editable_scenario
```

### Running Executables: PowerShell vs Git Bash

If your prompt looks like `PS C:\...>`, you are in PowerShell. Use backslashes:

```powershell
.\build\professor_smoke_test.exe
.\build\professor_editable_scenario.exe
.\build\report_results.exe
```

If your prompt looks like `$`, you are usually in Git Bash. Use forward slashes:

```bash
./build/professor_smoke_test.exe
./build/professor_editable_scenario.exe
./build/report_results.exe
```

Do not use `.\build\...` in Git Bash: Bash treats backslashes as escape characters.

### If CMake Cannot Find a Make Program on Windows

If CMake reports generator/tool errors, force the make path and compiler explicitly:

```bash
cmake -S . -B build -G "Unix Makefiles" \
  -DCMAKE_MAKE_PROGRAM="C:/PROGRA~2/GnuWin32/bin/make.exe" \
  -DCMAKE_CXX_COMPILER="/c/Users/<YOUR_USER>/Documents/CODE/GNAT/2021/bin/g++.exe"
```

Replace `<YOUR_USER>` with your Windows username.

---

### Fallback Method: Manual Scripts

If you do not have CMake configured, you can use the provided helper scripts from the root directory:

**On Windows (CMD/PowerShell):**
```bash
.\build_number_generation_app.bat
.\build\manual\number_generation_app.exe
```

**On Linux or macOS (Bash):**
```bash
bash build_number_generation_app.sh
./build/manual/number_generation_app.exe
```

**Verify Tests (Bash only):**
To instantly compile and verify all distributions without CMake:
```bash
bash verify.sh
```

Manual scripts now place generated executables in `build/manual/` to keep the project root and `tests/` folders clean.
Manual scripts also place object files in `build/manual/obj/` so root is not flooded with `.o` files.

## Troubleshooting

### Error: `Running 'nmake' '-?' failed`

Cause: CMake selected an NMake generator/environment, but Visual Studio build tools are not installed in the current shell.

Fix:

```bash
cmake -S . -B build -G "Unix Makefiles"
```

### Error: `CMake was unable to find a build program corresponding to "MinGW Makefiles"`

Cause: `mingw32-make` is not installed or not on `PATH`.

Fix options:

- Install `mingw32-make` and retry MinGW Makefiles.
- Or use `Unix Makefiles` with an available `make` executable.

### Error involving `/usr/bin/sh` and `C:/Program Files (x86)/.../make.exe`

Cause: parenthesis/spaces in the make path can break shell command parsing.

Fix:

Use the short DOS path in `CMAKE_MAKE_PROGRAM`:

```bash
-DCMAKE_MAKE_PROGRAM="C:/PROGRA~2/GnuWin32/bin/make.exe"
```

### Root directory full of `.o` files

Cause: ad-hoc manual compilation without `-c -o` object output paths.

Fix:

- Use CMake build (recommended).
- Or use `build_number_generation_app.sh` / `build_number_generation_app.bat`
  (already configured to write objects to `build/manual/obj/`).

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

## Minimal Usage Example

```cpp
#include <iostream>
#include "UniformGenerator/LinearCongruential.h"
#include "DiscreteGenerator/Bernoulli.h"

int main() {
  LinearCongruential uniform(42, 1103515245, 12345, 2147483648);
  Bernoulli bernoulli(0.7, &uniform);

  for (int i = 0; i < 5; ++i) {
    std::cout << bernoulli.Generate() << "\n";
  }

  std::cout << "Empirical mean: " << bernoulli.Mean(10000) << "\n";
  return 0;
}
```

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

## Notes

- Most non-uniform generators take a pointer to a `UniformGenerator` instance.
- `Mean(nbSim)` is implemented in the base class `RandomGenerator` and can be used on all derived generators returning scalar values.
