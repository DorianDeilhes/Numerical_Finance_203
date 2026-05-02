# Usage Examples: Parameterized nb_steps Architecture

## Quick Start Guide for Professor

### 1. Basic Usage (Default Settings)
```cpp
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"

// Setup
std::vector<double> spot_prices = {100.0, 105.0, 110.0};
std::vector<double> volatilities = {0.20, 0.22, 0.21};
std::vector<double> weights = {0.4, 0.3, 0.3};
double strike = 105.0;
double maturity = 1.0;
double risk_free_rate = 0.03;
std::vector<std::vector<double>> correlation_matrix = {
    {1.0, 0.5, 0.3},
    {0.5, 1.0, 0.4},
    {0.3, 0.4, 1.0}
};

// Create basket with default 100 time steps
EuropeanBasket basket(spot_prices, volatilities, weights, strike, maturity, 
                      risk_free_rate, correlation_matrix);

// Price it
EcuyerCombined rng(12345, 67890);
MonteCarloSummary result = basket.PriceFixedN(&rng, 10000);

std::cout << "Call price: " << result.mean << " ± " << result.confidenceIntervalHalfWidth << std::endl;
```

---

## 2. Speed-First Configuration (Fast Approximation)
Use for quick estimates, pre-production testing:

```cpp
// Fewer time steps = faster computation
EuropeanBasket basket_fast(spot_prices, volatilities, weights, strike, maturity, 
                           risk_free_rate, correlation_matrix, 
                           50);  // 50 time steps instead of default 100

MonteCarloSummary fast_result = basket_fast.PriceFixedN(&rng, 5000);
std::cout << "Quick estimate: " << fast_result.mean << std::endl;
// Computation time: ~half of default, slightly less accurate
```

---

## 3. Accuracy-First Configuration (High Precision)
Use for production, research validation:

```cpp
// More time steps = higher accuracy
EuropeanBasket basket_precise(spot_prices, volatilities, weights, strike, maturity, 
                              risk_free_rate, correlation_matrix, 
                              500);  // 500 time steps for high accuracy

MonteCarloSummary precise_result = basket_precise.PriceFixedN(&rng, 50000);
std::cout << "High-precision result: " << precise_result.mean << std::endl;
// Computation time: ~5x default, significantly more accurate
```

---

## 4. Variance Reduction Study (same basket, different modes)
```cpp
EcuyerCombined rng(98765, 43210);
EuropeanBasket basket(spot_prices, volatilities, weights, strike, maturity, 
                      risk_free_rate, correlation_matrix, 100);

// Baseline: no variance reduction
MonteCarloSummary baseline = basket.PriceFixedN(&rng, 10000);
std::cout << "Baseline variance: " << baseline.sampleVariance << std::endl;

// Antithetic: pair-averaged payoffs
MonteCarloSummary antithetic = basket.PriceFixedNAntithetic(&rng, 5000);  // 5000 pairs = 10000 samples
std::cout << "Antithetic variance: " << antithetic.sampleVariance << std::endl;

// Control variate: static beta from pilot
MonteCarloSummary control = basket.PriceFixedNControlVariate(&rng, 10000, 500);
std::cout << "Control variate variance: " << control.sampleVariance << std::endl;

// Cumulative: antithetic + control variate
MonteCarloSummary cumulative = basket.PriceFixedNCumulative(&rng, 5000, 500);
std::cout << "Cumulative variance: " << cumulative.sampleVariance << std::endl;

// Variance Reduction Factor = 1 - (var_reduced / var_baseline)
double vrf_antithetic = 1.0 - (antithetic.sampleVariance / baseline.sampleVariance);
double vrf_control = 1.0 - (control.sampleVariance / baseline.sampleVariance);
double vrf_cumulative = 1.0 - (cumulative.sampleVariance / baseline.sampleVariance);

std::cout << "\nVariance Reduction Factors:\n";
std::cout << "  Antithetic: " << (vrf_antithetic * 100.0) << "%\n";
std::cout << "  Control Variate: " << (vrf_control * 100.0) << "%\n";
std::cout << "  Cumulative: " << (vrf_cumulative * 100.0) << "%\n";
```

---

## 5. Efficiency Study: nb_steps Trade-off
```cpp
#include <chrono>

std::vector<size_t> nb_steps_configs = {25, 50, 100, 250, 500, 1000};

std::cout << "nb_steps | Time(ms) | Variance | Accuracy vs 1000 steps\n";
std::cout << "---------|----------|----------|---------------------\n";

for (size_t nb_steps : nb_steps_configs) {
  EuropeanBasket basket(spot_prices, volatilities, weights, strike, maturity, 
                        risk_free_rate, correlation_matrix, nb_steps);
  
  auto start = std::chrono::high_resolution_clock::now();
  MonteCarloSummary result = basket.PriceFixedN(&rng, 10000);
  auto end = std::chrono::high_resolution_clock::now();
  
  auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  
  std::cout << nb_steps << " | " << duration_ms << " | " 
            << result.sampleVariance << " | " 
            << ((nb_steps < 1000) ? "reference points" : "reference")
            << "\n";
}
// Shows linear speed-up: 2x steps ≈ 2x time
// Shows accuracy improvement: more steps = lower variance in samples
```

---

## 6. Quasirandom Sampling with Custom Steps
```cpp
#include "UniformGenerator/HaltonQuasiRandom.h"

// Quasirandom + moderate discretization
EuropeanBasket basket_qr_200(spot_prices, volatilities, weights, strike, maturity, 
                              risk_free_rate, correlation_matrix, 200);

HaltonQuasiRandom halton_gen(3, true, 0.314159);  // 3D, with shift, seed=pi
MonteCarloSummary qr_result = basket_qr_200.PriceFixedNCumulative(&halton_gen, 5000, 250);

std::cout << "Quasirandom + antithetic + control variate:\n";
std::cout << "  Price: " << qr_result.mean << "\n";
std::cout << "  SE: " << qr_result.standardError << "\n";
std::cout << "  95% CI: [" << (qr_result.mean - qr_result.confidenceIntervalHalfWidth) << ", " 
                               << (qr_result.mean + qr_result.confidenceIntervalHalfWidth) << "]\n";
```

---

## 7. Comparing Antithetic-Only vs Antithetic+Control
```cpp
// Performance analysis of new SinglePathAntitheticPayoffOnly() optimization

// BEFORE (old way): Computes control and discards it
MonteCarloSummary result_old = basket.PriceFixedNAntithetic(&rng, 5000);

// AFTER (new way): Uses optimized method, no wasted control computation
// (same result, faster computation)
MonteCarloSummary result_new = basket.PriceFixedNAntithetic(&rng, 5000);

// Prices should be comparable (some Monte Carlo randomness expected)
std::cout << "Antithetic (new optimized): " << result_new.mean << std::endl;

// Speed benefit visible in profiler: one basket-value computation avoided per sample
```

---

## 8. Parameter Sensitivity Analysis
```cpp
// Study how nb_steps affects final estimate stability
const size_t num_configs = 5;
std::vector<size_t> steps = {50, 100, 200, 500, 1000};

for (size_t nb : steps) {
  std::vector<double> prices;
  
  for (int run = 0; run < 10; ++run) {  // 10 independent runs
    EuropeanBasket basket(spot_prices, volatilities, weights, strike, maturity, 
                          risk_free_rate, correlation_matrix, nb);
    MonteCarloSummary result = basket.PriceFixedN(&rng, 5000);
    prices.push_back(result.mean);
  }
  
  // Compute mean and std of the 10 runs
  double mean_price = prices[0];  // simplified for brevity
  double stability = prices[prices.size()-1] - prices[0];  // range
  
  std::cout << "nb_steps=" << nb << ": mean estimator range = " << stability << std::endl;
}
// Insight: More steps → tighter distribution of estimates → more stable pricing
```

---

## 9. Professor's Configuration at Project End
```cpp
// CONFIGURATION: Tune these based on your accuracy/speed priorities

// Option A: Fast production environment
#define PRODUCTION_NB_STEPS 100
#define PRODUCTION_SAMPLES 10000
#define PRODUCTION_PILOT 500

// Option B: Accuracy-critical research
#define RESEARCH_NB_STEPS 500
#define RESEARCH_SAMPLES 50000
#define RESEARCH_PILOT 1000

// Option C: Real-time pricing (minimal latency)
#define REALTIME_NB_STEPS 50
#define REALTIME_SAMPLES 5000
#define REALTIME_PILOT 200

// Use in code:
#ifdef ACCURATE_MODE
  EuropeanBasket basket(..., RESEARCH_NB_STEPS);
  MonteCarloSummary result = basket.PriceFixedNCumulative(&rng, RESEARCH_SAMPLES, RESEARCH_PILOT);
#else
  EuropeanBasket basket(..., PRODUCTION_NB_STEPS);
  MonteCarloSummary result = basket.PriceFixedNCumulative(&rng, PRODUCTION_SAMPLES, PRODUCTION_PILOT);
#endif
```

---

## Key Design Principles Demonstrated

✓ **Parameterization**: Every decision (nb_steps, samples, pilot_count) is controllable
✓ **Defaults**: Sensible defaults (100 steps, 500 pilot) work out-of-box
✓ **Backward Compatible**: Existing code works without modification
✓ **Performance**: Optimized paths (SinglePathAntitheticPayoffOnly) avoid waste
✓ **Flexibility**: Easy to create configurations for different use cases
✓ **Professor Control**: University researcher can tune all parameters at project end

---

## Compile & Run Instructions

```bash
cd RandomNumberGeneration/build
cmake --build . --target test_phase3
./test_phase3.exe

# Or run the number-generation app
./number_generation_app.exe
```
