#pragma once

#include <array>

namespace PricingHelper {

// Solve a 3x3 linear system for the quadratic regression normal equations.
bool SolveLinearSystem3x3(const std::array<std::array<double, 3>, 3>& matrix,
                          const std::array<double, 3>& rhs,
                          std::array<double, 3>* solution);

}  // namespace PricingHelper
