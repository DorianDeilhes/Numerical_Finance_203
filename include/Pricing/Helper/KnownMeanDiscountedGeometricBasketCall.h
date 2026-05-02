#pragma once

#include <vector>

namespace PricingHelper {

// Analytic value of exp(-rT) E[(prod_i S_i(T)^alpha_i - K)^+].
double KnownMeanDiscountedGeometricBasketCall(
    const std::vector<double>& spot_prices,
    const std::vector<double>& volatilities,
    const std::vector<double>& weights,
    double strike,
    double maturity,
    double rate,
    const std::vector<std::vector<double>>& correlation_matrix);

}  // namespace PricingHelper
