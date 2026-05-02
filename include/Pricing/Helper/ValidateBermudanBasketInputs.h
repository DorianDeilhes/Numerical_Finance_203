#pragma once

#include <cstddef>
#include <vector>

namespace PricingHelper {

// Validate Bermudan-specific contract inputs, including exercise dates and step count.
void ValidateBermudanBasketInputs(const std::vector<double>& spot_prices,
                                  const std::vector<double>& volatilities,
                                  const std::vector<double>& weights,
                                  double strike,
                                  double maturity,
                                  double risk_free_rate,
                                  const std::vector<std::vector<double>>& correlation_matrix,
                                  const std::vector<double>& exercise_dates,
                                  const std::vector<double>& dividend_yields,
                                  size_t nb_steps);

}  // namespace PricingHelper
