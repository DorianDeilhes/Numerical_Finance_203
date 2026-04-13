#include "Pricing/Helper/BasketValue.h"

#include <stdexcept>

namespace PricingHelper {

double BasketValue(const std::vector<double>& spots, const std::vector<double>& weights) {
  if (spots.size() != weights.size()) {
    throw std::runtime_error("BasketValue requires spots and weights with matching sizes");
  }

  double value = 0.0;
  // Weighted linear basket B = sum_i w_i S_i.
  for (size_t i = 0; i < spots.size(); ++i) {
    value += weights[i] * spots[i];
  }
  return value;
}

}  // namespace PricingHelper
