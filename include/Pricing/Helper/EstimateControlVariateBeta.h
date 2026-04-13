#pragma once

#include <vector>

namespace PricingHelper {

double EstimateControlVariateBeta(const std::vector<double>& targetSamples,
                                  const std::vector<double>& controlSamples);

}  // namespace PricingHelper
