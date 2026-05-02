# Quick Reference

Short cheat sheet for compiling, testing, and running the project.
For explanations and examples, use `README.md` and `USAGE_EXAMPLES.md`.

## Build And Test

Run from the project root:

```bash
cmake -S . -B build
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Low-RAM build:

```bash
cmake --build build
```

## Run On Windows

Git Bash:

```bash
./build/professor_smoke_test.exe
./build/professor_editable_scenario.exe
./build/report_results.exe
./build/number_generation_app.exe
```

PowerShell:

```powershell
.\build\professor_smoke_test.exe
.\build\professor_editable_scenario.exe
.\build\report_results.exe
.\build\number_generation_app.exe
```

## Run On Linux Or macOS

```bash
./build/professor_smoke_test
./build/professor_editable_scenario
./build/report_results
./build/number_generation_app
```

## Executables

| Executable | Use |
|---|---|
| `professor_smoke_test` | Fixed demo to check that pricing works. |
| `professor_editable_scenario` | Main file for custom products and pricer settings. |
| `report_results` | Creates local `report_results.csv` for tables and graphs. |
| `number_generation_app` | Original random generator demo, not the final pricing app. |

## Custom Scenario

Edit only these two functions in `professor_editable_scenario.cpp`:

```cpp
BuildProductToModify()
BuildPricerConfigToModify()
```

Then rebuild and run:

```bash
cmake --build build --target professor_editable_scenario --parallel 2
./build/professor_editable_scenario.exe
```

## Product Settings

In `BuildProductToModify()`:

| Setting | Meaning |
|---|---|
| `exercise_style` | `EuropeanStyle` or `BermudanStyle`. |
| `payoff_type` | Current choice: `BasketCallPayoff`. |
| `spot_prices` | Initial asset prices. Basket dimension is this vector size. |
| `volatilities` | One annual volatility per asset. |
| `weights` | Basket weights. Negative weights are allowed. |
| `strike` | Strike `K`. |
| `maturity` | Maturity `T`, in years. |
| `risk_free_rate` | Constant risk-free rate `r`. |
| `correlation_matrix` | Square correlation matrix matching the basket dimension. |
| `exercise_dates` | Bermudan exercise dates. First date may be `0.0`; last date must be maturity. Ignored for European products. |

## Pricer Settings

In `BuildPricerConfigToModify()`:

| Setting | Meaning |
|---|---|
| `pricing_method` | `BasicMonteCarlo`, `StaticControlVariate`, `AntitheticVariables`, or `CumulativeVarianceReduction`. Control-variate modes require non-negative weights summing to `1`. |
| `random_generator` | `PseudoRandom` or `QuasiRandom`. |
| `nb_steps` | Black-Scholes time steps. Bermudan dates must align with this grid. |
| `path_count` | Number of paths for basic and control-variate pricing. |
| `pair_count` | Number of direct/antithetic pairs. |
| `pilot_count` | Pilot paths used to estimate the control-variate coefficient. |
| `pseudo_seed_1`, `pseudo_seed_2` | Seeds for pseudo-random simulations. |
| `halton_dimension` | Halton dimension. Use `0` for automatic dimension. |
| `use_halton_shift`, `halton_shift_seed` | Optional shifted Halton sequence settings. |

## Report Order

`report_results` compares:

1. basic pseudo-random Monte Carlo,
2. quasi-random Monte Carlo,
3. quasi-random + static control variate,
4. quasi-random + static control variate + antithetic variables.

Run:

```bash
./build/report_results.exe
```
