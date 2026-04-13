#pragma once

#include <vector>

namespace PricingHelper {

double KnownMeanDiscountedBasketValue(const std::vector<double>& spotPrices,
                                      const std::vector<double>& weights);

}  // namespace PricingHelper
