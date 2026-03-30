#include "PDEFunctions/PutBottomBoundary.h"

PutBottomBoundary::PutBottomBoundary(double sMin, double strike)
    : sMin_(sMin), strike_(strike) {}

double PutBottomBoundary::operator()(double t) {
  (void)t;

  if (strike_ > sMin_) {
    return strike_ - sMin_;
  }
  return 0.0;
}

PutBottomBoundary::~PutBottomBoundary() {}
