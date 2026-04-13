#pragma once

#include <cstddef>
#include <vector>

namespace PricingHelper {

// Map contractual exercise dates onto the discrete simulation grid.
std::vector<size_t> ConvertExerciseDatesToStepIndices(const std::vector<double>& exercise_dates,
                                                      double maturity,
                                                      size_t nb_steps);

}  // namespace PricingHelper
