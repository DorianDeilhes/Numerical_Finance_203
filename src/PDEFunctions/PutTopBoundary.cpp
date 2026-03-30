#include "PDEFunctions/PutTopBoundary.h"

PutTopBoundary::PutTopBoundary(double sMax, double strike)
    : sMax_(sMax), strike_(strike) {}

double PutTopBoundary::operator()(double t) {
  (void)t;

  if (strike_ > sMax_) {
    return strike_ - sMax_;
  }
  return 0.0;
}

PutTopBoundary::~PutTopBoundary() {}
