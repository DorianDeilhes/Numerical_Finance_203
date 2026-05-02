#include "Pricing/Helper/ValidateGeometricBasketControlWeights.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

void ValidateGeometricBasketControlWeights(const std::vector<double>& weights) {
  if (weights.empty()) {
    throw std::invalid_argument(
        "Geometric basket control variate requires at least one weight");
  }

  const double tolerance = 1e-10;
  double sum = 0.0;
  for (size_t i = 0; i < weights.size(); ++i) {
    if (!std::isfinite(weights[i])) {
      throw std::invalid_argument(
          "Geometric basket control variate requires finite weights");
    }
    if (weights[i] < -tolerance) {
      throw std::invalid_argument(
          "Geometric basket control variate requires non-negative weights");
    }
    sum += weights[i];
  }

  if (std::fabs(sum - 1.0) > tolerance) {
    throw std::invalid_argument(
        "Geometric basket control variate requires weights summing to 1");
  }
}

}  // namespace PricingHelper
