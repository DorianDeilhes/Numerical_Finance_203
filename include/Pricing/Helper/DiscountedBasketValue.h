#pragma once

#include <vector>

namespace PricingHelper {

double DiscountedBasketValue(const std::vector<double>& terminalSpots,
                             const std::vector<double>& weights,
                             double rate,
                             double maturity);

}  // namespace PricingHelper
