// test_large_basket_stress.cpp
//
// Stress tests that exercise the loading-matrix decomposition and the full
// pricing pipeline at basket dimensions far beyond the typical 2–3 asset case.
// Path counts are intentionally tiny (6–30 paths) because the goal is to verify
// numerical stability of construction and one pricing step, not pricing accuracy.
//
// Scenarios:
//   10-asset equicorrelation (ρ = 0.25) — Cholesky succeeds on a well-conditioned
//     positive-definite matrix; checks that EuropeanBasket returns a finite result
//   50-asset block correlation — two blocks with within-block ρ = 0.30 and
//     between-block ρ = 0.05; exercises BermudanBasket at scale with 2 exercise dates
//   100-asset identity correlation — Cholesky trivial (diagonal); the main stress
//     here is memory allocation and the Halton / Gaussian sampling loop at D=100
//   Near-singular (30-asset ρ = 0.999) — Cholesky is numerically borderline;
//     BuildLoadingMatrixFromCorrelation must still return a finite square matrix
//     (it falls back to Jacobi eigendecomposition if Cholesky fails)
//   Singular PSD (8-asset ρ = 1.0) — Cholesky fails; Jacobi fallback is required;
//     verifies that the fallback path produces a finite loading matrix

#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "Pricing/Helper/BuildLoadingMatrixFromCorrelation.h"
#include "UniformGenerator/EcuyerCombined.h"

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

void CheckFiniteSummary(const MonteCarloSummary& summary, const std::string& label) {
  Check(std::isfinite(summary.mean), label + " mean must be finite");
  Check(std::isfinite(summary.sampleVariance), label + " variance must be finite");
  Check(std::isfinite(summary.standardError), label + " standard error must be finite");
  Check(summary.sampleSize >= 2, label + " sample size must be >= 2");
}

std::vector<double> ConstantVector(size_t dimension, double value) {
  return std::vector<double>(dimension, value);
}

std::vector<double> EqualWeights(size_t dimension) {
  return std::vector<double>(dimension, 1.0 / static_cast<double>(dimension));
}

std::vector<double> MildlyVaryingSpots(size_t dimension) {
  std::vector<double> spots(dimension, 100.0);
  for (size_t i = 0; i < dimension; ++i) {
    spots[i] = 95.0 + static_cast<double>(i % 11);
  }
  return spots;
}

std::vector<double> MildlyVaryingVolatilities(size_t dimension) {
  std::vector<double> volatilities(dimension, 0.20);
  for (size_t i = 0; i < dimension; ++i) {
    volatilities[i] = 0.15 + 0.01 * static_cast<double>(i % 6);
  }
  return volatilities;
}

std::vector<std::vector<double>> IdentityCorrelation(size_t dimension) {
  std::vector<std::vector<double>> correlation(dimension,
                                               std::vector<double>(dimension, 0.0));
  for (size_t i = 0; i < dimension; ++i) {
    correlation[i][i] = 1.0;
  }
  return correlation;
}

std::vector<std::vector<double>> Equicorrelation(size_t dimension, double rho) {
  std::vector<std::vector<double>> correlation(dimension,
                                               std::vector<double>(dimension, rho));
  for (size_t i = 0; i < dimension; ++i) {
    correlation[i][i] = 1.0;
  }
  return correlation;
}

std::vector<std::vector<double>> BlockCorrelation(size_t dimension,
                                                  double within_block_rho,
                                                  double between_block_rho) {
  std::vector<std::vector<double>> correlation(
      dimension, std::vector<double>(dimension, between_block_rho));
  const size_t split = dimension / 2;

  for (size_t i = 0; i < dimension; ++i) {
    for (size_t j = 0; j < dimension; ++j) {
      const bool same_first_block = (i < split && j < split);
      const bool same_second_block = (i >= split && j >= split);
      if (same_first_block || same_second_block) {
        correlation[i][j] = within_block_rho;
      }
    }
    correlation[i][i] = 1.0;
  }

  return correlation;
}

void TestTenAssetEquicorrelationEuropean() {
  const size_t dimension = 10;
  EuropeanBasket basket(MildlyVaryingSpots(dimension),
                        MildlyVaryingVolatilities(dimension),
                        EqualWeights(dimension),
                        95.0,
                        1.0,
                        0.03,
                        Equicorrelation(dimension, 0.25),
                        /*nb_steps=*/8);

  EcuyerCombined rng(12345, 67890);
  const MonteCarloSummary summary = basket.PriceFixedN(&rng, 30);
  CheckFiniteSummary(summary, "10-asset equicorrelation European");
}

void TestFiftyAssetBlockCorrelationBermudan() {
  const size_t dimension = 50;
  BermudanBasket basket(MildlyVaryingSpots(dimension),
                        MildlyVaryingVolatilities(dimension),
                        EqualWeights(dimension),
                        95.0,
                        1.0,
                        0.03,
                        BlockCorrelation(dimension, 0.30, 0.05),
                        /*exercise_dates=*/{0.5, 1.0},
                        /*nb_steps=*/4);

  EcuyerCombined rng(24680, 13579);
  const MonteCarloSummary summary = basket.PriceFixedN(&rng, 10);
  CheckFiniteSummary(summary, "50-asset block-correlation Bermudan");
}

void TestHundredAssetIdentityEuropean() {
  const size_t dimension = 100;
  EuropeanBasket basket(MildlyVaryingSpots(dimension),
                        ConstantVector(dimension, 0.20),
                        EqualWeights(dimension),
                        95.0,
                        1.0,
                        0.03,
                        IdentityCorrelation(dimension),
                        /*nb_steps=*/3);

  EcuyerCombined rng(11111, 22222);
  const MonteCarloSummary summary = basket.PriceFixedN(&rng, 6);
  CheckFiniteSummary(summary, "100-asset identity-correlation European");
}

void TestNearSingularLargeCorrelationAccepted() {
  const size_t dimension = 30;
  const std::vector<std::vector<double>> correlation =
      Equicorrelation(dimension, 0.999);

  const std::vector<std::vector<double>> loading_matrix =
      PricingHelper::BuildLoadingMatrixFromCorrelation(correlation);

  Check(loading_matrix.size() == dimension,
        "Near-singular loading matrix must keep requested dimension");
  for (size_t i = 0; i < dimension; ++i) {
    Check(loading_matrix[i].size() == dimension,
          "Near-singular loading matrix must be square");
    for (size_t j = 0; j < dimension; ++j) {
      Check(std::isfinite(loading_matrix[i][j]),
            "Near-singular loading matrix entries must be finite");
    }
  }
}

void TestSingularPositiveSemidefiniteFallbackAccepted() {
  const size_t dimension = 8;
  const std::vector<std::vector<double>> correlation =
      Equicorrelation(dimension, 1.0);

  const std::vector<std::vector<double>> loading_matrix =
      PricingHelper::BuildLoadingMatrixFromCorrelation(correlation);

  Check(loading_matrix.size() == dimension,
        "Singular PSD loading matrix must keep requested dimension");
  for (size_t i = 0; i < dimension; ++i) {
    Check(loading_matrix[i].size() == dimension,
          "Singular PSD loading matrix must be square");
    for (size_t j = 0; j < dimension; ++j) {
      Check(std::isfinite(loading_matrix[i][j]),
            "Singular PSD loading matrix entries must be finite");
    }
  }
}

}  // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  PHASE 6 LARGE BASKET STRESS TEST SUITE\n";
    std::cout << "============================================================\n\n";

    TestTenAssetEquicorrelationEuropean();
    TestFiftyAssetBlockCorrelationBermudan();
    TestHundredAssetIdentityEuropean();
    TestNearSingularLargeCorrelationAccepted();
    TestSingularPositiveSemidefiniteFallbackAccepted();

    std::cout << "[PHASE 6 STRESS TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[PHASE 6 STRESS TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
