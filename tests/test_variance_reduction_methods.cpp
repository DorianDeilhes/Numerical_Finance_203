// test_variance_reduction_methods.cpp
//
// Basic tests confirming that all four variance reduction layers run end-to-end
// on a 2-asset European basket without producing NaN, Inf, or negative prices.
// These tests do NOT verify numerical accuracy or variance ordering — that is
// handled by test_dividend_pricing.cpp (TestVarianceReductionOrdering).
// The purpose here is purely integration: each code path (quasi-random
// generator wiring, control variate β estimation, antithetic mirroring, and
// their combination) must complete without throwing and must return a finite,
// positive price.
//
// Reference basket: S = {100, 105}, σ = {0.20, 0.25}, w = {0.60, 0.40},
//                   K = 100, T = 1, r = 0.03, ρ = 0.35, nb_steps = 100
//
// Methods tested:
//   1. PriceFixedN with HaltonQuasiRandom (quasi-random baseline)
//   2. PriceFixedNControlVariate with EcuyerCombined (geometric basket CV)
//   3. PriceFixedNAntithetic with EcuyerCombined (antithetic pairs)
//   4. PriceFixedNCumulative with HaltonQuasiRandom (quasi + CV + antithetic)

#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/HaltonQuasiRandom.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void Check(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void CheckFinite(const MonteCarloSummary& summary, const std::string& label) {
  Check(std::isfinite(summary.mean), label + " mean must be finite");
  Check(std::isfinite(summary.sampleVariance), label + " variance must be finite");
  Check(std::isfinite(summary.standardError), label + " std error must be finite");
  Check(summary.sampleSize >= 2, label + " sample size must be >= 2");
}

EuropeanBasket BuildReferenceBasket(size_t nb_steps = 100) {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.2, 0.25};
  std::vector<double> weight = {0.6, 0.4};
  double strike = 100.0;
  double maturity = 1.0;
  double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
  return EuropeanBasket(spot, vol, weight, strike, maturity, rate, corr, nb_steps);
}

void TestQuasiRandomGeneratorIntegrates() {
  EuropeanBasket basket = BuildReferenceBasket();
  HaltonQuasiRandom halton(/*dimension=*/256, /*use shift=*/true, /*seed=*/0.314159);
  const MonteCarloSummary summary = basket.PriceFixedN(&halton, 1000);
  CheckFinite(summary, "QuasiRandom baseline");
  Check(summary.mean > 0.0, "QuasiRandom baseline price must be positive");
}

void TestControlVariatePricingRuns() {
  EuropeanBasket basket = BuildReferenceBasket();
  EcuyerCombined rng(12345, 67890);
  const MonteCarloSummary summary = basket.PriceFixedNControlVariate(&rng, 1500, 400);
  CheckFinite(summary, "ControlVariate");
  Check(summary.mean > 0.0, "ControlVariate price must be positive");
}

void TestAntitheticPricingRuns() {
  EuropeanBasket basket = BuildReferenceBasket();
  EcuyerCombined rng(98765, 43210);
  const MonteCarloSummary summary = basket.PriceFixedNAntithetic(&rng, 1200);
  CheckFinite(summary, "Antithetic");
  Check(summary.mean > 0.0, "Antithetic price must be positive");
}

void TestCumulativePricingRuns() {
  EuropeanBasket basket = BuildReferenceBasket();
  HaltonQuasiRandom halton(/*dimension=*/256, /*use shift=*/true, /*seed=*/0.271828);
  const MonteCarloSummary summary = basket.PriceFixedNCumulative(&halton, 1200, 350);
  CheckFinite(summary, "Cumulative");
  Check(summary.mean > 0.0, "Cumulative price must be positive");
}

}  // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  PHASE 3 TEST SUITE\n";
    std::cout << "============================================================\n\n";

    TestQuasiRandomGeneratorIntegrates();
    TestControlVariatePricingRuns();
    TestAntitheticPricingRuns();
    TestCumulativePricingRuns();

    std::cout << "[PHASE 3 TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[PHASE 3 TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
