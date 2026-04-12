#pragma once

#include <cstddef>
#include <vector>

namespace PricingHelper {

void ValidateCorrelationMatrix(const std::vector<std::vector<double>>& correlationMatrix,
                               size_t dimension);

}  // namespace PricingHelper