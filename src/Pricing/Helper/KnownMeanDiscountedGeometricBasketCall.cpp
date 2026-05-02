#include "Pricing/Helper/KnownMeanDiscountedGeometricBasketCall.h"

#include "Pricing/Helper/ValidateCorrelationMatrix.h"
#include "Pricing/Helper/ValidateGeometricBasketControlWeights.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

double StandardNormalCdf(double x) {
  return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

}  // namespace

namespace PricingHelper {

double KnownMeanDiscountedGeometricBasketCall(
    const std::vector<double>& spot_prices,
    const std::vector<double>& volatilities,
    const std::vector<double>& weights,
    double strike,
    double maturity,
    double rate,
    const std::vector<std::vector<double>>& correlation_matrix) {
  const size_t dimension = spot_prices.size();
  if (dimension == 0) {
    throw std::runtime_error(
        "KnownMeanDiscountedGeometricBasketCall requires dimension > 0");
  }
  if (volatilities.size() != dimension || weights.size() != dimension) {
    throw std::runtime_error(
        "KnownMeanDiscountedGeometricBasketCall requires matching input sizes");
  }
  ValidateGeometricBasketControlWeights(weights);
  ValidateCorrelationMatrix(correlation_matrix, dimension);

  if (!std::isfinite(strike) || strike < 0.0) {
    throw std::runtime_error(
        "KnownMeanDiscountedGeometricBasketCall requires strike >= 0");
  }
  if (!std::isfinite(maturity) || maturity < 0.0) {
    throw std::runtime_error(
        "KnownMeanDiscountedGeometricBasketCall requires maturity >= 0");
  }
  if (!std::isfinite(rate)) {
    throw std::runtime_error(
        "KnownMeanDiscountedGeometricBasketCall requires finite rate");
  }

  double log_geometric_spot = 0.0;
  double mu_log = 0.0;
  for (size_t i = 0; i < dimension; ++i) {
    if (!std::isfinite(spot_prices[i]) || spot_prices[i] <= 0.0) {
      throw std::runtime_error(
          "KnownMeanDiscountedGeometricBasketCall requires positive finite spots");
    }
    if (!std::isfinite(volatilities[i]) || volatilities[i] < 0.0) {
      throw std::runtime_error(
          "KnownMeanDiscountedGeometricBasketCall requires non-negative finite volatilities");
    }

    log_geometric_spot += weights[i] * std::log(spot_prices[i]);
    mu_log += weights[i] * (rate - 0.5 * volatilities[i] * volatilities[i]);
  }

  double variance_rate = 0.0;
  for (size_t i = 0; i < dimension; ++i) {
    for (size_t j = 0; j < dimension; ++j) {
      variance_rate += weights[i] * weights[j] * volatilities[i] *
                       volatilities[j] * correlation_matrix[i][j];
    }
  }

  const double tolerance = 1e-12;
  if (variance_rate < -tolerance) {
    throw std::runtime_error(
        "KnownMeanDiscountedGeometricBasketCall computed negative variance");
  }
  variance_rate = std::max(variance_rate, 0.0);

  const double geometric_spot = std::exp(log_geometric_spot);
  const double discount = std::exp(-rate * maturity);
  const double expected_geometric =
      geometric_spot * std::exp((mu_log + 0.5 * variance_rate) * maturity);

  if (strike == 0.0) {
    return discount * expected_geometric;
  }

  const double variance = variance_rate * maturity;
  if (variance <= tolerance) {
    const double deterministic_geometric =
        geometric_spot * std::exp(mu_log * maturity);
    return discount * std::max(deterministic_geometric - strike, 0.0);
  }

  const double stddev = std::sqrt(variance);
  const double d1 =
      (std::log(geometric_spot / strike) + (mu_log + variance_rate) * maturity) /
      stddev;
  const double d2 = d1 - stddev;

  return discount *
         (expected_geometric * StandardNormalCdf(d1) -
          strike * StandardNormalCdf(d2));
}

}  // namespace PricingHelper
