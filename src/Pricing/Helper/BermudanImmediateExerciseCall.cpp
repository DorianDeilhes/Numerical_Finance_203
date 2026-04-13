#include "Pricing/Helper/BermudanImmediateExerciseCall.h"

namespace PricingHelper {

double BermudanImmediateExerciseCall(double basket_value, double strike) {
  // Intrinsic value of a basket call at an exercise date.
  const double intrinsic = basket_value - strike;
  return intrinsic > 0.0 ? intrinsic : 0.0;
}

}  // namespace PricingHelper
