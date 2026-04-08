#include "PDEFunctions/CallBottomBoundary.h"
#include <algorithm>
#include <cmath>

CallBottomBoundary::CallBottomBoundary(double sMin, double strike, double rate,
                                       double maturity)
    : sMin_(sMin), strike_(strike), rate_(rate), maturity_(maturity) {}

double CallBottomBoundary::operator()(double t) {
  const double tau = std::max(maturity_ - t, 0.0);
  const double discountedStrike = strike_ * std::exp(-rate_ * tau);
  return std::max(sMin_ - discountedStrike, 0.0);
}

CallBottomBoundary::~CallBottomBoundary() {}
