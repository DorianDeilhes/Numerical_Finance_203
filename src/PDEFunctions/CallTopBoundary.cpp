#include "PDEFunctions/CallTopBoundary.h"

CallTopBoundary::CallTopBoundary(double sMax, double strike)
    : sMax_(sMax), strike_(strike) {}

double CallTopBoundary::operator()(double t) {
  (void)t;

  if (sMax_ > strike_) {
    return sMax_ - strike_;
  }
  return 0.0;
}

CallTopBoundary::~CallTopBoundary() {}
