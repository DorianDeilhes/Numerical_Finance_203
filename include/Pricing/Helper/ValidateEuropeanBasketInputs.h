#pragma once

#include <vector>

namespace PricingHelper {

void ValidateEuropeanBasketInputs(
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    const std::vector<double>& weights,
    double strike,
    double maturity,
    double riskFreeRate,
    const std::vector<std::vector<double>>& correlationMatrix);

}  // namespace PricingHelper
