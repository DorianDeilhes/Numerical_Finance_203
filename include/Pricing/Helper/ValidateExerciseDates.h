#pragma once

#include <vector>

namespace PricingHelper {

void ValidateExerciseDates(const std::vector<double>& exercise_dates, double maturity);

}  // namespace PricingHelper
