#pragma once

#include <array>

namespace PricingHelper {

// Evaluate the fitted continuation polynomial at one basket state.
double EvaluateQuadraticContinuation(double state,
                                     const std::array<double, 3>& coefficients);

}  // namespace PricingHelper
