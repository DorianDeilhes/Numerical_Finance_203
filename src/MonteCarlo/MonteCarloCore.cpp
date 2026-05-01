#include "MonteCarlo/MonteCarloCore.h"

#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {

// Online moments via Welford updates for numerical stability.
struct OnlineMoments {
  size_t n;
  double mean;
  double m2;
};

void UpdateMoments(double sample, OnlineMoments* moments) {
  moments->n += 1;
  const double delta = sample - moments->mean;
  moments->mean += delta / static_cast<double>(moments->n);
  const double delta2 = sample - moments->mean;
  moments->m2 += delta * delta2;
}

void ValidateConfidenceLevel(double confidenceLevel) {
  if (!(confidenceLevel > 0.0 && confidenceLevel < 1.0)) {
    throw std::runtime_error("MonteCarloCore requires confidenceLevel in (0, 1)");
  }
}

void ValidateSampleValue(double sample) {
  if (!std::isfinite(sample)) {
    throw std::runtime_error("MonteCarloCore sampleGenerator returned a non-finite value");
  }
}

} // namespace

MonteCarloSummary MonteCarloCore::RunFixedN(
    const std::function<double()>& sampleGenerator,
    size_t sampleSize,
    double confidenceLevel) {
  ValidateConfidenceLevel(confidenceLevel);
  if (!sampleGenerator) {
    throw std::runtime_error("MonteCarloCore::RunFixedN requires a valid sample generator");
  }
  if (sampleSize < 2) {
    throw std::runtime_error("MonteCarloCore::RunFixedN requires sampleSize >= 2");
  }

  OnlineMoments moments = {0, 0.0, 0.0};
  for (size_t k = 0; k < sampleSize; ++k) {
    const double sample = sampleGenerator();
    ValidateSampleValue(sample);
    UpdateMoments(sample, &moments);
  }

  return BuildSummary(moments.n, moments.mean, moments.m2, confidenceLevel);
}

MonteCarloSummary MonteCarloCore::RunFixedPrecision(
    const std::function<double()>& sampleGenerator,
    double epsilon,
    double confidenceLevel,
    size_t minSamples,
    size_t maxSamples) {
  ValidateConfidenceLevel(confidenceLevel);
  if (!sampleGenerator) {
    throw std::runtime_error(
        "MonteCarloCore::RunFixedPrecision requires a valid sample generator");
  }
  if (!(epsilon > 0.0) || !std::isfinite(epsilon)) {
    throw std::runtime_error("MonteCarloCore::RunFixedPrecision requires finite epsilon > 0");
  }
  if (minSamples < 2) {
    throw std::runtime_error("MonteCarloCore::RunFixedPrecision requires minSamples >= 2");
  }
  if (maxSamples < minSamples) {
    throw std::runtime_error(
        "MonteCarloCore::RunFixedPrecision requires maxSamples >= minSamples");
  }

  OnlineMoments moments = {0, 0.0, 0.0};
  for (size_t k = 0; k < maxSamples; ++k) {
    const double sample = sampleGenerator();
    ValidateSampleValue(sample);
    UpdateMoments(sample, &moments);

    if (moments.n >= minSamples) {
      const MonteCarloSummary summary =
          BuildSummary(moments.n, moments.mean, moments.m2, confidenceLevel);
      if (summary.confidenceInterval.halfWidth <= epsilon) {
        return summary;
      }
    }
  }

  return BuildSummary(moments.n, moments.mean, moments.m2, confidenceLevel);
}

std::string MonteCarloCore::BuildReport(const MonteCarloSummary& summary) {
  std::ostringstream out;
  out.precision(10);
  out << "N=" << summary.sampleSize
      << ", mean=" << summary.mean
      << ", var=" << summary.sampleVariance
      << ", stderr=" << summary.standardError
      << ", CI(" << summary.confidenceInterval.confidenceLevel << ")=["
      << summary.confidenceInterval.lower << ", "
      << summary.confidenceInterval.upper << "]"
      << ", halfWidth=" << summary.confidenceInterval.halfWidth;
  return out.str();
}

MonteCarloSummary MonteCarloCore::BuildSummary(size_t sampleSize,
                                               double mean,
                                               double m2,
                                               double confidenceLevel) {
  if (sampleSize < 2) {
    throw std::runtime_error("MonteCarloCore requires at least two samples to estimate variance");
  }

  MonteCarloSummary summary;
  summary.sampleSize = sampleSize;
  summary.mean = mean;
  summary.sampleVariance = m2 / static_cast<double>(sampleSize - 1);
  summary.standardError =
      std::sqrt(summary.sampleVariance / static_cast<double>(sampleSize));
  summary.confidenceInterval =
      BuildConfidenceInterval(summary.sampleSize, summary.mean,
                              summary.standardError, confidenceLevel);
  return summary;
}

double MonteCarloCore::InverseStandardNormalCDF(double probability) {
  if (!(probability > 0.0 && probability < 1.0)) {
    throw std::runtime_error("InverseStandardNormalCDF requires probability in (0, 1)");
  }

  // Acklam rational approximation.
  static const double a1 = -3.969683028665376e+01;
  static const double a2 = 2.209460984245205e+02;
  static const double a3 = -2.759285104469687e+02;
  static const double a4 = 1.383577518672690e+02;
  static const double a5 = -3.066479806614716e+01;
  static const double a6 = 2.506628277459239e+00;

  static const double b1 = -5.447609879822406e+01;
  static const double b2 = 1.615858368580409e+02;
  static const double b3 = -1.556989798598866e+02;
  static const double b4 = 6.680131188771972e+01;
  static const double b5 = -1.328068155288572e+01;

  static const double c1 = -7.784894002430293e-03;
  static const double c2 = -3.223964580411365e-01;
  static const double c3 = -2.400758277161838e+00;
  static const double c4 = -2.549732539343734e+00;
  static const double c5 = 4.374664141464968e+00;
  static const double c6 = 2.938163982698783e+00;

  static const double d1 = 7.784695709041462e-03;
  static const double d2 = 3.224671290700398e-01;
  static const double d3 = 2.445134137142996e+00;
  static const double d4 = 3.754408661907416e+00;

  static const double pLow = 0.02425;
  static const double pHigh = 1.0 - pLow;

  if (probability < pLow) {
    const double q = std::sqrt(-2.0 * std::log(probability));
    return (((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) /
           ((((d1 * q + d2) * q + d3) * q + d4) * q + 1.0);
  }

  if (probability > pHigh) {
    const double q = std::sqrt(-2.0 * std::log(1.0 - probability));
    return -(((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) /
            ((((d1 * q + d2) * q + d3) * q + d4) * q + 1.0);
  }

  const double q = probability - 0.5;
  const double r = q * q;
  return (((((a1 * r + a2) * r + a3) * r + a4) * r + a5) * r + a6) * q /
         (((((b1 * r + b2) * r + b3) * r + b4) * r + b5) * r + 1.0);
}

ConfidenceInterval MonteCarloCore::BuildConfidenceInterval(size_t sampleSize,
                                                           double mean,
                                                           double standardError,
                                                           double confidenceLevel) {
  if (sampleSize < 2) {
    throw std::runtime_error("BuildConfidenceInterval requires sampleSize >= 2");
  }
  if (!(confidenceLevel > 0.0 && confidenceLevel < 1.0)) {
    throw std::runtime_error("BuildConfidenceInterval requires confidenceLevel in (0, 1)");
  }
  if (!(standardError >= 0.0) || !std::isfinite(standardError)) {
    throw std::runtime_error("BuildConfidenceInterval requires a finite non-negative standardError");
  }

  const double tailProbability = 0.5 * (1.0 + confidenceLevel);
  const double zScore = InverseStandardNormalCDF(tailProbability);
  const double halfWidth = zScore * standardError;

  ConfidenceInterval interval;
  interval.lower = mean - halfWidth;
  interval.upper = mean + halfWidth;
  interval.halfWidth = halfWidth;
  interval.confidenceLevel = confidenceLevel;
  interval.zScore = zScore;
  return interval;
}
