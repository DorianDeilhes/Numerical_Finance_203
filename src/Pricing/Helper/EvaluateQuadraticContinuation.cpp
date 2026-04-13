#include "Pricing/Helper/EvaluateQuadraticContinuation.h"

namespace PricingHelper {

double EvaluateQuadraticContinuation(double state,
                                     const std::array<double, 3>& coefficients) {
  // C_hat(B) = a0 + a1*B + a2*B^2.
  return coefficients[0] + coefficients[1] * state + coefficients[2] * state * state;
}

}  // namespace PricingHelper
