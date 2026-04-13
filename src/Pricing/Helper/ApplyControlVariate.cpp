#include "Pricing/Helper/ApplyControlVariate.h"

namespace PricingHelper {

double ApplyControlVariate(double targetSample,
                           double controlSample,
                           double controlMean,
                           double beta) {
  return targetSample - beta * (controlSample - controlMean);
}

}  // namespace PricingHelper
