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
- Windows: PowerShell + MinGW/MSYS2 (or similar) for `g++`
- Optional: Git Bash/WSL to run `verify.sh`

## Build and Run

### Primary Method: CMake (Recommended)

This project uses **CMake** as its primary build system, ensuring cross-platform compatibility identical to modern C++ standards.

```bash
# 1. Create a build directory
mkdir build
cd build

# 2. Generate the build files
# (If you are on Windows using MinGW, use: cmake -G "Unix Makefiles" ..)
cmake ..

# 3. Compile the executables
cmake --build .
```

After compilation, run your simulator from the `build/` folder:
```bash
./interactive_sim.exe
```

To run the automated mathematical verification tests natively, execute:
```bash
ctest --output-on-failure
```

---

### Fallback Method: Manual Scripts

If you do not have CMake configured, you can use the provided helper scripts from the root directory:

**On Windows (CMD/PowerShell):**
```bash
.\build_interactive.bat
.\interactive_sim.exe
```

**On Linux or macOS (Bash):**
```bash
bash build_interactive.sh
./interactive_sim.exe
```

**Verify Tests (Bash only):**
To instantly compile and verify all distributions without CMake:
```bash
bash verify.sh
```

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