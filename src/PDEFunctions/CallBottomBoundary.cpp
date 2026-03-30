#include "PDEFunctions/CallBottomBoundary.h"

CallBottomBoundary::CallBottomBoundary(double sMin, double strike)
    : sMin_(sMin), strike_(strike) {}

double CallBottomBoundary::operator()(double t) {
  (void)t;

  if (sMin_ > strike_) {
    return sMin_ - strike_;
  }
  return 0.0;
}

CallBottomBoundary::~CallBottomBoundary() {}
