#include "ContinuousGenerator/Normal.h"
#include "MonteCarlo/MonteCarloCore.h"
#include "UniformGenerator/EcuyerCombined.h"

#include <cmath>
#include <functional>
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

void TestFixedNDeterministicSequence() {
  std::vector<double> samples = {1.0, 2.0, 3.0, 4.0};
  size_t index = 0;
  const std::function<double()> sampler = [&samples, &index]() {
    const double value = samples[index];
    ++index;
    return value;
  };

  const MonteCarloSummary summary = MonteCarloCore::RunFixedN(sampler, samples.size(), 0.95);
  Check(std::fabs(summary.mean - 2.5) < 1e-12, "FixedN mean mismatch");
  Check(std::fabs(summary.sampleVariance - (5.0 / 3.0)) < 1e-12,
        "FixedN variance mismatch");
  const double expectedStandardError = std::sqrt((5.0 / 3.0) / 4.0);
  Check(std::fabs(summary.standardError - expectedStandardError) < 1e-12,
        "FixedN standard error mismatch");
  Check(summary.confidenceInterval.upper > summary.confidenceInterval.lower,
        "FixedN CI bounds are invalid");
}

void TestFixedPrecisionConstantSampler() {
  const std::function<double()> sampler = []() { return 3.0; };
  const MonteCarloSummary summary = MonteCarloCore::RunFixedPrecision(
      sampler, 1e-6, 0.95, 20, 10000);

  Check(summary.sampleSize == 20,
        "FixedPrecision should stop at minSamples for zero-variance sampler");
  Check(std::fabs(summary.mean - 3.0) < 1e-12,
        "FixedPrecision mean mismatch for constant sampler");
  Check(std::fabs(summary.sampleVariance) < 1e-12,
        "FixedPrecision variance should be zero for constant sampler");
  Check(std::fabs(summary.confidenceInterval.halfWidth) < 1e-12,
        "FixedPrecision CI should collapse for constant sampler");
}

void TestFixedPrecisionNormalSampler() {
  EcuyerCombined uniform(12345, 67890);
  Normal normal(0.0, 1.0, BoxMuller, &uniform);
  const std::function<double()> sampler = [&normal]() { return normal.Generate(); };

  const MonteCarloSummary summary = MonteCarloCore::RunFixedPrecision(
      sampler, 0.25, 0.95, 50, 5000);

  Check(summary.sampleSize >= 50, "FixedPrecision sample size below minSamples");
  Check(summary.sampleSize <= 5000, "FixedPrecision exceeded maxSamples");
  Check(summary.confidenceInterval.halfWidth <= 0.25 + 1e-12,
        "FixedPrecision did not meet epsilon target");
}

void TestValidationErrors() {
  const std::function<double()> emptySampler;
  ExpectThrows("RunFixedN must reject empty samplers", [&emptySampler]() {
    MonteCarloCore::RunFixedN(emptySampler, 10, 0.95);
  });

  ExpectThrows("RunFixedN must reject sampleSize < 2", []() {
    MonteCarloCore::RunFixedN([]() { return 1.0; }, 1, 0.95);
  });

  ExpectThrows("RunFixedN must reject invalid confidence level", []() {
    MonteCarloCore::RunFixedN([]() { return 1.0; }, 10, 1.0);
  });

  ExpectThrows("RunFixedPrecision must reject epsilon <= 0", []() {
    MonteCarloCore::RunFixedPrecision([]() { return 1.0; }, 0.0, 0.95, 10, 100);
  });

  ExpectThrows("RunFixedPrecision must reject minSamples < 2", []() {
    MonteCarloCore::RunFixedPrecision([]() { return 1.0; }, 0.1, 0.95, 1, 100);
  });

  ExpectThrows("RunFixedPrecision must reject maxSamples < minSamples", []() {
    MonteCarloCore::RunFixedPrecision([]() { return 1.0; }, 0.1, 0.95, 100, 50);
  });

  ExpectThrows("RunFixedN must reject non-finite samples", []() {
    MonteCarloCore::RunFixedN([]() { return std::nan(""); }, 10, 0.95);
  });
}

} // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  MONTE CARLO CORE TEST SUITE\n";
    std::cout << "============================================================\n\n";

    TestFixedNDeterministicSequence();
    TestFixedPrecisionConstantSampler();
    TestFixedPrecisionNormalSampler();
    TestValidationErrors();

    std::cout << "[MC CORE TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[MC CORE TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
