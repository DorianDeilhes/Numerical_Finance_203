#include "Pricing/Helper/KnownMeanDiscountedBasketValue.h"

#include <stdexcept>

namespace PricingHelper {

double KnownMeanDiscountedBasketValue(const std::vector<double>& spotPrices,
                                      const std::vector<double>& weights) {
  if (spotPrices.size() != weights.size()) {
    throw std::runtime_error(
        "KnownMeanDiscountedBasketValue requires matching spot and weight sizes");
  }

  double mean = 0.0;
  for (size_t i = 0; i < spotPrices.size(); ++i) {
    mean += weights[i] * spotPrices[i];
  }
  return mean;
}

}  // namespace PricingHelper
