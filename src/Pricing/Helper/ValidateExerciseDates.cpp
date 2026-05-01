#include "Pricing/Helper/ValidateExerciseDates.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

void ValidateExerciseDates(const std::vector<double>& exercise_dates, double maturity) {
  if (!std::isfinite(maturity)) {
    throw std::invalid_argument("BermudanBasket: maturity must be finite");
  }
  if (maturity <= 0.0) {
    throw std::invalid_argument("BermudanBasket: maturity must be positive");
  }
  if (exercise_dates.empty()) {
    throw std::invalid_argument("BermudanBasket: exercise dates must not be empty");
  }

  // Exercise schedule must be strictly increasing and within [0, T].
  for (size_t i = 0; i < exercise_dates.size(); ++i) {
    const double t = exercise_dates[i];
    if (!std::isfinite(t)) {
      throw std::invalid_argument("BermudanBasket: exercise dates must be finite");
    }
    if (t < 0.0) {
      throw std::invalid_argument("BermudanBasket: exercise dates must be >= 0");
    }
    if (i > 0 && !(t > 0.0)) {
      throw std::invalid_argument("BermudanBasket: only the first exercise date may be 0");
    }
    if (t - maturity > 1e-12) {
      throw std::invalid_argument("BermudanBasket: exercise dates must be <= maturity");
    }
    if (i > 0 && !(exercise_dates[i - 1] < t)) {
      throw std::invalid_argument("BermudanBasket: exercise dates must be strictly increasing");
    }
  }

  if (std::fabs(exercise_dates.back() - maturity) > 1e-12) {
    // This baseline implementation assumes final exercise is at maturity.
    throw std::invalid_argument("BermudanBasket: last exercise date must equal maturity");
  }
}

}  // namespace PricingHelper
