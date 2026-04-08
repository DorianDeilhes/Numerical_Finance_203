#include "PDEFunctions/PutBottomBoundary.h"
#include <algorithm>
#include <cmath>

PutBottomBoundary::PutBottomBoundary(double sMin, double strike, double rate,
                                     double maturity)
    : sMin_(sMin), strike_(strike), rate_(rate), maturity_(maturity) {}

double PutBottomBoundary::operator()(double t) {
  const double tau = std::max(maturity_ - t, 0.0);
  const double discountedStrike = strike_ * std::exp(-rate_ * tau);
  return std::max(discountedStrike - sMin_, 0.0);
}

PutBottomBoundary::~PutBottomBoundary() {}
