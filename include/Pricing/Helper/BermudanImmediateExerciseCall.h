#pragma once

namespace PricingHelper {

// Immediate exercise payoff for a Bermudan basket call.
double BermudanImmediateExerciseCall(double basket_value, double strike);

}  // namespace PricingHelper
