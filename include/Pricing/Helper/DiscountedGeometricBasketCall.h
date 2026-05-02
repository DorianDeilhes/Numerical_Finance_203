#pragma once

#include <vector>

namespace PricingHelper {

// Y_j = exp(-rT) * (prod_i S_i(T)^alpha_i - K)^+.
double DiscountedGeometricBasketCall(const std::vector<double>& terminal_spots,
                                     const std::vector<double>& weights,
                                     double strike,
                                     double rate,
                                     double maturity);

}  // namespace PricingHelper
