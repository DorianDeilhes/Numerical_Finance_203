#pragma once

#include <cstddef>
#include <vector>

namespace PricingHelper {

void ValidateEuropeanBasketInputs(
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    const std::vector<double>& weights,
    double strike,
    double maturity,
    double riskFreeRate,
    const std::vector<std::vector<double>>& correlationMatrix,
    const std::vector<double>& dividendYields,
    size_t nbSteps);

}  // namespace PricingHelper
