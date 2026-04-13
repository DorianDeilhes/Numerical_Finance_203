#pragma once

#include <array>
#include <vector>

namespace PricingHelper {

// Fit quadratic continuation coefficients on (state, discounted target) samples.
bool EstimateQuadraticContinuationCoefficients(const std::vector<double>& states,
                                               const std::vector<double>& targets,
                                               std::array<double, 3>* coefficients);

}  // namespace PricingHelper
