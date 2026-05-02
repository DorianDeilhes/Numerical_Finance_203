#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void Check(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename Callable>
void ExpectThrows(const std::string& message, Callable&& callable) {
  bool threw = false;
  try {
    callable();
  } catch (const std::exception&) {
    threw = true;
  }
  Check(threw, message);
}

void TestOneDimensionalBasket() {
  // 1D basket: single asset, weight=1, is just a European call on that asset.
  std::vector<double> spot = {100.0};
  std::vector<double> vol = {0.2};
  std::vector<double> weight = {1.0};
  double strike = 100.0;
  double maturity = 1.0;
  double rate = 0.05;
  std::vector<std::vector<double>> corr = {{1.0}};

  EuropeanBasket basket(spot, vol, weight, strike, maturity, rate, corr);

  EcuyerCombined uniform(12345, 67890);
  const MonteCarloSummary summary = basket.PriceFixedN(&uniform, 5000);

  // Sanity check: price should be positive and reasonable.
  Check(summary.mean > 0.0, "1D basket price should be positive");
  Check(summary.mean < spot[0], "1D basket call price should be < spot");
  Check(summary.standardError > 0.0, "1D basket should have positive std error");
  Check(summary.confidenceInterval.upper > summary.confidenceInterval.lower,
        "1D basket CI bounds invalid");
}

void TestTwoDimensionalBasket() {
  // 2D basket: two correlated assets with equal weights.
  std::vector<double> spot = {100.0, 100.0};
  std::vector<double> vol = {0.2, 0.2};
  std::vector<double> weight = {0.5, 0.5};
  double strike = 100.0;
  double maturity = 1.0;
  double rate = 0.05;
  std::vector<std::vector<double>> corr = {{1.0, 0.5}, {0.5, 1.0}};

  EuropeanBasket basket(spot, vol, weight, strike, maturity, rate, corr);

  EcuyerCombined uniform(11111, 22222);
  const MonteCarloSummary summary = basket.PriceFixedN(&uniform, 5000);

  // Sanity check: price should be positive and reasonable.
  Check(summary.mean > 0.0, "2D basket price should be positive");
  Check(summary.mean < spot[0], "2D basket call price should be < spot");
  Check(summary.confidenceInterval.upper > summary.confidenceInterval.lower,
        "2D basket CI bounds invalid");
}

void TestReproducibility() {
  // Same seed should give same results (deterministic pseudo-random).
  std::vector<double> spot = {100.0, 100.0};
  std::vector<double> vol = {0.2, 0.2};
  std::vector<double> weight = {0.5, 0.5};
  double strike = 100.0;
  double maturity = 1.0;
  double rate = 0.05;
  std::vector<std::vector<double>> corr = {{1.0, 0.3}, {0.3, 1.0}};

  EuropeanBasket basket(spot, vol, weight, strike, maturity, rate, corr);

  // First run.
  EcuyerCombined uniform1(99999, 88888);
  const MonteCarloSummary summary1 = basket.PriceFixedN(&uniform1, 1000);

  // Second run with same seed.
  EcuyerCombined uniform2(99999, 88888);
  const MonteCarloSummary summary2 = basket.PriceFixedN(&uniform2, 1000);

  // Means should match exactly (same seed, same sequence).
  Check(std::fabs(summary1.mean - summary2.mean) < 1e-10,
        "Reproducibility test failed: means differ with same seed");
}

void TestFixedPrecisionMode() {
  // Test adaptive sampling until epsilon target met.
  std::vector<double> spot = {100.0};
  std::vector<double> vol = {0.2};
  std::vector<double> weight = {1.0};
  double strike = 100.0;
  double maturity = 1.0;
  double rate = 0.05;
  std::vector<std::vector<double>> corr = {{1.0}};

  EuropeanBasket basket(spot, vol, weight, strike, maturity, rate, corr);

  EcuyerCombined uniform(55555, 44444);
  const MonteCarloSummary summary = basket.PriceFixedPrecision(&uniform, 0.5, 50, 10000);

  Check(summary.sampleSize >= 50, "FixedPrecision should respect minSamples");
  Check(summary.sampleSize <= 10000, "FixedPrecision should respect maxSamples");
  Check(summary.confidenceInterval.halfWidth <= 0.5 + 1e-12,
        "FixedPrecision did not meet epsilon target");
}

void TestValidationErrors() {
  // Dimension mismatch: weights size != spot size.
  ExpectThrows("Should reject weights size mismatch", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5}, 100.0, 1.0, 0.05,
                          {{1.0, 0.3}, {0.3, 1.0}});
  });

  // Dimension mismatch: volatilities size != spot size.
  ExpectThrows("Should reject volatilities size mismatch", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2}, {0.5, 0.5}, 100.0, 1.0, 0.05,
                          {{1.0, 0.3}, {0.3, 1.0}});
  });

  // Negative spot price.
  ExpectThrows("Should reject negative spot price", []() {
    EuropeanBasket basket({-100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5}, 100.0, 1.0, 0.05,
                          {{1.0, 0.3}, {0.3, 1.0}});
  });

  // Negative volatility.
  ExpectThrows("Should reject negative volatility", []() {
    EuropeanBasket basket({100.0, 100.0}, {-0.2, 0.2}, {0.5, 0.5}, 100.0, 1.0, 0.05,
                          {{1.0, 0.3}, {0.3, 1.0}});
  });

  // Negative maturity.
  ExpectThrows("Should reject non-positive maturity", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5}, 100.0, -1.0, 0.05,
                          {{1.0, 0.3}, {0.3, 1.0}});
  });

  // Correlation entry out of bounds.
  ExpectThrows("Should reject correlation outside [-1, 1]", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5}, 100.0, 1.0, 0.05,
                          {{1.0, 1.5}, {1.5, 1.0}});
  });

  // Correlation matrix not symmetric.
  ExpectThrows("Should reject asymmetric correlation matrix", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5}, 100.0, 1.0, 0.05,
                          {{1.0, 0.3}, {0.5, 1.0}});
  });

  // Correlation diagonal not 1.
  ExpectThrows("Should reject correlation diagonal != 1", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5}, 100.0, 1.0, 0.05,
                          {{0.95, 0.3}, {0.3, 1.0}});
  });

  // Non-positive-semidefinite correlation matrix.
  ExpectThrows("Should reject non-PSD correlation matrix", []() {
    EuropeanBasket basket({100.0, 100.0, 100.0}, {0.2, 0.2, 0.2},
                          {0.3, 0.3, 0.4}, 100.0, 1.0, 0.05,
                          {{1.0, 0.9, 0.9},
                           {0.9, 1.0, -0.9},
                           {0.9, -0.9, 1.0}});
  });

  // Zero simulation step count.
  ExpectThrows("Should reject nb_steps == 0", []() {
    EuropeanBasket basket({100.0}, {0.2}, {1.0}, 100.0, 1.0, 0.05, {{1.0}}, 0);
  });

  // Zero dimension.
  ExpectThrows("Should reject zero dimension", []() {
    EuropeanBasket basket({}, {}, {}, 100.0, 1.0, 0.05, {});
  });

  // Non-finite spot price.
  ExpectThrows("Should reject NaN spot price", []() {
    EuropeanBasket basket({std::numeric_limits<double>::quiet_NaN()}, {0.2}, {1.0},
                          100.0, 1.0, 0.05, {{1.0}});
  });

  // Non-finite volatility.
  ExpectThrows("Should reject infinite volatility", []() {
    EuropeanBasket basket({100.0}, {std::numeric_limits<double>::infinity()}, {1.0},
                          100.0, 1.0, 0.05, {{1.0}});
  });

  // Non-finite weight.
  ExpectThrows("Should reject NaN weight", []() {
    EuropeanBasket basket({100.0}, {0.2}, {std::numeric_limits<double>::quiet_NaN()},
                          100.0, 1.0, 0.05, {{1.0}});
  });

  // Non-finite scalar inputs.
  ExpectThrows("Should reject infinite strike", []() {
    EuropeanBasket basket({100.0}, {0.2}, {1.0},
                          std::numeric_limits<double>::infinity(), 1.0, 0.05, {{1.0}});
  });

  ExpectThrows("Should reject NaN risk-free rate", []() {
    EuropeanBasket basket({100.0}, {0.2}, {1.0}, 100.0, 1.0,
                          std::numeric_limits<double>::quiet_NaN(), {{1.0}});
  });

  // Wrong matrix shape and non-finite matrix entry.
  ExpectThrows("Should reject non-square correlation matrix", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5},
                          100.0, 1.0, 0.05, {{1.0}, {0.2, 1.0}});
  });

  ExpectThrows("Should reject NaN correlation entry", []() {
    EuropeanBasket basket({100.0, 100.0}, {0.2, 0.2}, {0.5, 0.5},
                          100.0, 1.0, 0.05,
                          {{1.0, std::numeric_limits<double>::quiet_NaN()},
                           {std::numeric_limits<double>::quiet_NaN(), 1.0}});
  });
}

void TestPublicPricingValidationErrors() {
  EuropeanBasket basket({100.0}, {0.2}, {1.0}, 100.0, 1.0, 0.05, {{1.0}});
  EcuyerCombined uniform(12345, 67890);

  ExpectThrows("Should reject null generator in PriceFixedN", [&]() {
    (void)basket.PriceFixedN(nullptr, 10);
  });

  ExpectThrows("Should reject sample count < 2 in PriceFixedN", [&]() {
    (void)basket.PriceFixedN(&uniform, 1);
  });

  ExpectThrows("Should reject infinite epsilon in PriceFixedPrecision", [&]() {
    (void)basket.PriceFixedPrecision(&uniform, std::numeric_limits<double>::infinity(), 10, 20);
  });

  ExpectThrows("Should reject pair count < 2 in antithetic", [&]() {
    (void)basket.PriceFixedNAntithetic(&uniform, 1);
  });

  ExpectThrows("Should reject sample count < 2 in control variate", [&]() {
    (void)basket.PriceFixedNControlVariate(&uniform, 1, 10);
  });

  ExpectThrows("Should reject pilot count < 2 in cumulative", [&]() {
    (void)basket.PriceFixedNCumulative(&uniform, 10, 1);
  });

  EuropeanBasket negative_weight_basket({100.0, 105.0}, {0.2, 0.25},
                                        {0.7, -0.2}, 100.0, 1.0, 0.05,
                                        {{1.0, 0.3}, {0.3, 1.0}}, 20);
  ExpectThrows("Control variate should reject negative weights", [&]() {
    EcuyerCombined rng(11111, 22222);
    (void)negative_weight_basket.PriceFixedNControlVariate(&rng, 10, 5);
  });

  EuropeanBasket non_normalized_weight_basket({100.0, 105.0}, {0.2, 0.25},
                                              {0.7, 0.2}, 100.0, 1.0, 0.05,
                                              {{1.0, 0.3}, {0.3, 1.0}}, 20);
  ExpectThrows("Control variate should reject weights not summing to 1", [&]() {
    EcuyerCombined rng(33333, 44444);
    (void)non_normalized_weight_basket.PriceFixedNCumulative(&rng, 10, 5);
  });
}

}  // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  EUROPEAN BASKET TEST SUITE\n";
    std::cout << "============================================================\n\n";

    TestOneDimensionalBasket();
    TestTwoDimensionalBasket();
    TestReproducibility();
    TestFixedPrecisionMode();
    TestValidationErrors();
    TestPublicPricingValidationErrors();

    std::cout << "[BASKET TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[BASKET TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
