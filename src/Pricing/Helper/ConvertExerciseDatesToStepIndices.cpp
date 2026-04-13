#include "Pricing/Helper/ConvertExerciseDatesToStepIndices.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

std::vector<size_t> ConvertExerciseDatesToStepIndices(const std::vector<double>& exercise_dates,
                                                      double maturity,
                                                      size_t nb_steps) {
  if (nb_steps == 0) {
    throw std::invalid_argument("ConvertExerciseDatesToStepIndices requires nb_steps > 0");
  }

  std::vector<size_t> step_indices;
  step_indices.reserve(exercise_dates.size());

  for (size_t i = 0; i < exercise_dates.size(); ++i) {
    // Convert continuous date t to the nearest grid index t * nb_steps / T.
    const double scaled = exercise_dates[i] * static_cast<double>(nb_steps) / maturity;
    const double rounded = std::round(scaled);

    // Reject dates that do not land on the simulation grid.
    if (std::fabs(scaled - rounded) > 1e-10) {
      throw std::invalid_argument(
          "BermudanBasket: exercise dates must align with the simulation time grid");
    }

    const size_t step = static_cast<size_t>(rounded);
    if (step == 0 || step > nb_steps) {
      throw std::invalid_argument("BermudanBasket: computed step index is outside valid range");
    }
    if (!step_indices.empty() && step_indices.back() >= step) {
      throw std::invalid_argument("BermudanBasket: exercise step indices must be strictly increasing");
    }

    step_indices.push_back(step);
  }

  return step_indices;
}

}  // namespace PricingHelper
