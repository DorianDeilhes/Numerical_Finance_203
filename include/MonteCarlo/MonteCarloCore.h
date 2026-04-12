#pragma once

#include <cstddef>
#include <functional>
#include <string>

// Two-sided confidence interval around a Monte Carlo estimator.
struct ConfidenceInterval {
  double lower;
  double upper;
  double halfWidth;
  double confidenceLevel;
  double zScore;
};

// Summary statistics returned by Monte Carlo runs.
struct MonteCarloSummary {
  size_t sampleSize;
  double mean;
  double sampleVariance;
  double standardError;
  ConfidenceInterval confidenceInterval;
};

// Core Monte Carlo utilities used by pricing layers.
class MonteCarloCore {
public:
  // Runs a fixed-size Monte Carlo simulation.
  static MonteCarloSummary RunFixedN(const std::function<double()>& sampleGenerator,
                                     size_t sampleSize,
                                     double confidenceLevel = 0.95);

  // Runs until confidence interval half-width <= epsilon, or maxSamples is reached.
  static MonteCarloSummary RunFixedPrecision(
      const std::function<double()>& sampleGenerator,
      double epsilon,
      double confidenceLevel = 0.95,
      size_t minSamples = 30,
      size_t maxSamples = 1000000);

  // Returns a compact report string for logs and console output.
  static std::string BuildReport(const MonteCarloSummary& summary);

private:
  // Builds summary statistics from online moments.
  static MonteCarloSummary BuildSummary(size_t sampleSize,
                                        double mean,
                                        double m2,
                                        double confidenceLevel);

  // Inverse CDF of the standard normal distribution.
  static double InverseStandardNormalCDF(double probability);

  // Computes two-sided confidence interval from summary statistics.
  static ConfidenceInterval BuildConfidenceInterval(size_t sampleSize,
                                                    double mean,
                                                    double standardError,
                                                    double confidenceLevel);
};
