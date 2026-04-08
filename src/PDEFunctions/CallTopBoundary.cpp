#include "PDEFunctions/CallTopBoundary.h"
#include <algorithm>
#include <cmath>

CallTopBoundary::CallTopBoundary(double sMax, double strike, double rate,
                                 double maturity)
    : sMax_(sMax), strike_(strike), rate_(rate), maturity_(maturity) {}

double CallTopBoundary::operator()(double t) {
  const double tau = std::max(maturity_ - t, 0.0);
  const double discountedStrike = strike_ * std::exp(-rate_ * tau);
  return std::max(sMax_ - discountedStrike, 0.0);
}

CallTopBoundary::~CallTopBoundary() {}
