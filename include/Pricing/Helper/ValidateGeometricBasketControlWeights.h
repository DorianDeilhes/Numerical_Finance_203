#pragma once

#include <vector>

namespace PricingHelper {

// Lecture geometric basket control requires alpha_i >= 0 and sum alpha_i = 1.
void ValidateGeometricBasketControlWeights(const std::vector<double>& weights);

}  // namespace PricingHelper
