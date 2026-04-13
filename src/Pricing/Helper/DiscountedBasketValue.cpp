#include "Pricing/Helper/DiscountedBasketValue.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

double DiscountedBasketValue(const std::vector<double>& terminalSpots,
                             const std::vector<double>& weights,
                             double rate,
                             double maturity) {
  if (terminalSpots.size() != weights.size()) {
    throw std::runtime_error("DiscountedBasketValue requires matching spot and weight sizes");
  }

  double basketValue = 0.0;
  for (size_t i = 0; i < terminalSpots.size(); ++i) {
    basketValue += weights[i] * terminalSpots[i];
  }

  return std::exp(-rate * maturity) * basketValue;
}

}  // namespace PricingHelper
