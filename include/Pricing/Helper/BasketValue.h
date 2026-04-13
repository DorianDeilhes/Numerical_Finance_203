#pragma once

#include <vector>

namespace PricingHelper {

// Compute the weighted basket level from ND spots.
double BasketValue(const std::vector<double>& spots, const std::vector<double>& weights);

}  // namespace PricingHelper
