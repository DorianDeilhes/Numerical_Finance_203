# Random Number Generation Library (C++)

Small C++ library of pseudo-random generators for numerical finance coursework.

It provides:
- uniform pseudo-random generators,
- discrete distribution generators,
- continuous distribution generators,
- a bivariate normal generator,
- test programs to verify behavior and empirical moments.

## Project Structure

```text
.
|- include/
|  |- RandomGenerator.h
|  |- UniformGenerator/
|  |- DiscreteGenerator/
|  `- ContinuousGenerator/
|- src/
|  |- RandomGenerator.cpp
|  |- UniformGenerator/
|  |- DiscreteGenerator/
|  `- ContinuousGenerator/
|- interactive_sim.cpp
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

### Windows (Git Bash + g++)

Use this exact sequence from the project root:

```bash
rm -rf build
cmake -S . -B build -G "Unix Makefiles"
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Run the simulator:

```bash
./build/interactive_sim.exe
```

### Linux/macOS

From the project root:

```bash
rm -rf build
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Run the simulator:

```bash
./build/interactive_sim
```

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
.\build_interactive.bat
.\build\manual\interactive_sim.exe
```

**On Linux or macOS (Bash):**
```bash
bash build_interactive.sh
./build/manual/interactive_sim.exe
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
- Or use `build_interactive.sh` / `build_interactive.bat` (already configured to write objects to `build/manual/obj/`).

## Implemented Generators

### Uniform
- `LinearCongruential(seed, a, c, m)`
- `EcuyerCombined(seed1, seed2)`

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

The `tests/` directory contains several focused and aggregate test programs, including:
- `test_all_fixed.cpp`
- `test_all_generators.cpp`
- `test_continuous.cpp`
- `test_discrete.cpp`
- `test_lcg.cpp`
- `test_ecuyer.cpp`

Compile any test file by replacing the test source in the manual compile command.

## Notes

- Most non-uniform generators take a pointer to a `UniformGenerator` instance.
- `Mean(nbSim)` is implemented in the base class `RandomGenerator` and can be used on all derived generators returning scalar values.