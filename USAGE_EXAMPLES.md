# Usage Examples

This file shows the practical ways to use the project after it has been built with CMake.

Build once from the project root:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

## 1. Quick End-To-End Check

Run the fixed public demo:

```bash
./build/professor_smoke_test.exe
```

This runs European and Bermudan examples and prints results for:

- basic pseudo-random Monte Carlo,
- quasi-random Monte Carlo,
- static control variate,
- antithetic variables,
- cumulative variance reduction.

Use this when you want to check that the project works without editing inputs.

## 2. Main User Workflow

The main file to edit is:

```text
professor_editable_scenario.cpp
```

Only edit these two functions:

```cpp
BuildProductToModify()
BuildPricerConfigToModify()
```

After editing, rebuild only this executable:

```bash
cmake --build build --target professor_editable_scenario --parallel 2
```

Then run it:

```bash
./build/professor_editable_scenario.exe
```

## 3. Example Product Changes

### European Two-Asset Basket

In `BuildProductToModify()`:

```cpp
product.exercise_style = EuropeanStyle;
product.payoff_type = BasketCallPayoff;

product.spot_prices = {100.0, 105.0};
product.volatilities = {0.20, 0.25};
product.weights = {0.60, 0.40};
product.strike = 100.0;
product.maturity = 1.0;
product.risk_free_rate = 0.03;
product.dividend_yields = {0.0, 0.0};

product.correlation_matrix = {
    {1.0, 0.35},
    {0.35, 1.0}
};
```

For `EuropeanStyle`, `exercise_dates` is ignored.
The vector `dividend_yields` contains one non-negative continuous dividend
yield per asset. Its size must match `spot_prices`. Use `0.0` for assets
without dividends.

### Bermudan Two-Asset Basket

```cpp
product.exercise_style = BermudanStyle;
product.exercise_dates = {0.0, 0.25, 0.50, 0.75, 1.0};
```

The last date must be equal to `product.maturity`.
The first date may be `0.0`, matching the statement notation `t0 = 0`.
Dates must align with the simulation grid. For example, with:

```cpp
product.maturity = 1.0;
config.nb_steps = 40;
```

the time step is `1.0 / 40 = 0.025`, so `0.25`, `0.50`, `0.75`, and `1.0` are aligned.

### Three-Asset Basket With A Negative Weight

```cpp
product.spot_prices = {100.0, 95.0, 110.0};
product.volatilities = {0.18, 0.22, 0.30};
product.weights = {0.70, 0.50, -0.20};
product.strike = 102.0;
product.dividend_yields = {0.0, 0.0, 0.0};

product.correlation_matrix = {
    {1.0, 0.20, -0.10},
    {0.20, 1.0, 0.25},
    {-0.10, 0.25, 1.0}
};
```

The vectors and matrix must all match the same dimension.

Negative weights are accepted by the basket payoff itself. However, the
lecture geometric basket control variate requires non-negative weights summing
to `1`. For a product with negative weights, use `BasicMonteCarlo`,
`BasicMonteCarlo` with `QuasiRandom`, or `AntitheticVariables`.

## 4. Example Pricer Changes

In `BuildPricerConfigToModify()`:

### Basic Pseudo-Random Monte Carlo

```cpp
config.pricing_method = BasicMonteCarlo;
config.random_generator = PseudoRandom;
config.path_count = 10000;
```

### Quasi-Random Monte Carlo

```cpp
config.pricing_method = BasicMonteCarlo;
config.random_generator = QuasiRandom;
config.path_count = 10000;
config.halton_dimension = 0;
config.use_halton_shift = true;
```

### Static Control Variate

```cpp
config.pricing_method = StaticControlVariate;
config.random_generator = PseudoRandom;
config.path_count = 10000;
config.pilot_count = 500;
```

This uses the lecture geometric basket control:

```text
Y = exp(-rT) * (prod_i S_i(T)^alpha_i - K)^+
```

The analytic value of `E[Y]` is computed by the program. This method requires
all weights to be non-negative and to sum to `1`.
If dividend yields are non-zero, the analytic control value uses the same
dividend-adjusted Black-Scholes drift as the simulated basket paths.

### Antithetic Variables

```cpp
config.pricing_method = AntitheticVariables;
config.random_generator = PseudoRandom;
config.pair_count = 5000;
```

`pair_count = 5000` means 5000 direct/antithetic pairs, so 10000 paths are simulated.

### Cumulative Variance Reduction

```cpp
config.pricing_method = CumulativeVarianceReduction;
config.random_generator = QuasiRandom;
config.pair_count = 5000;
config.pilot_count = 500;
```

This is the main full variance reduction mode for the project.
It has the same weight restriction as `StaticControlVariate`.

## 5. Direct C++ API Example

You can also call the pricing classes directly.

```cpp
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"

#include <iostream>
#include <vector>

int main() {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.20, 0.25};
  std::vector<double> weights = {0.60, 0.40};
  std::vector<double> dividends = {0.0, 0.0};
  std::vector<std::vector<double>> corr = {
      {1.0, 0.35},
      {0.35, 1.0}
  };

  EuropeanBasket basket(spot, vol, weights,
                        100.0,  // strike
                        1.0,    // maturity
                        0.03,   // risk-free rate
                        corr,
                        40,     // nb_steps
                        dividends);

  EcuyerCombined rng(12345, 67890);
  MonteCarloSummary result = basket.PriceFixedN(&rng, 10000);

  std::cout << "Price: " << result.mean << "\n";
  std::cout << "Std error: " << result.standardError << "\n";
  std::cout << "95% CI: ["
            << result.confidenceInterval.lower << ", "
            << result.confidenceInterval.upper << "]\n";
}
```

## 6. Generate Report Data

Run:

```bash
./build/report_results.exe
```

This creates:

```text
report_results.csv
```

The CSV includes:

- price,
- sample variance,
- standard error,
- confidence interval,
- variance reduction factor,
- required sample-size gain.

Use this file for report tables and graphs.

## 7. Run The Number-Generation App

The original random-number-generation part of the project can be run with:

```bash
./build/number_generation_app.exe
```

This app is not the final pricing application. It is a console tool for trying the random generators and distributions.
