#include "Pricing/Helper/DiscountedGeometricBasketCall.h"

#include "Pricing/Helper/ValidateGeometricBasketControlWeights.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace PricingHelper {

double DiscountedGeometricBasketCall(const std::vector<double>& terminal_spots,
                                     const std::vector<double>& weights,
                                     double strike,
                                     double rate,
                                     double maturity) {
  if (terminal_spots.size() != weights.size()) {
    throw std::runtime_error(
        "DiscountedGeometricBasketCall requires matching spot and weight sizes");
  }
  ValidateGeometricBasketControlWeights(weights);

  if (!std::isfinite(strike) || strike < 0.0) {
    throw std::runtime_error("DiscountedGeometricBasketCall requires strike >= 0");
  }
  if (!std::isfinite(rate)) {
    throw std::runtime_error("DiscountedGeometricBasketCall requires finite rate");
  }
  if (!std::isfinite(maturity) || maturity < 0.0) {
    throw std::runtime_error("DiscountedGeometricBasketCall requires maturity >= 0");
  }

  double weighted_log_spot = 0.0;
  for (size_t i = 0; i < terminal_spots.size(); ++i) {
    if (!std::isfinite(terminal_spots[i]) || terminal_spots[i] <= 0.0) {
      throw std::runtime_error(
          "DiscountedGeometricBasketCall requires positive finite spots");
    }
    weighted_log_spot += weights[i] * std::log(terminal_spots[i]);
  }

  const double geometric_basket = std::exp(weighted_log_spot);
  if (!std::isfinite(geometric_basket)) {
    throw std::runtime_error("DiscountedGeometricBasketCall produced non-finite basket");
  }

  const double payoff = std::max(geometric_basket - strike, 0.0);
  return std::exp(-rate * maturity) * payoff;
}

}  // namespace PricingHelper
