#pragma once

namespace PricingHelper {

double ApplyControlVariate(double targetSample,
                           double controlSample,
                           double controlMean,
                           double beta);

}  // namespace PricingHelper
