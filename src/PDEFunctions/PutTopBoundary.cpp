#include "PDEFunctions/PutTopBoundary.h"
#include <algorithm>
#include <cmath>

PutTopBoundary::PutTopBoundary(double sMax, double strike, double rate,
                               double maturity)
    : sMax_(sMax), strike_(strike), rate_(rate), maturity_(maturity) {}

double PutTopBoundary::operator()(double t) {
  const double tau = std::max(maturity_ - t, 0.0);
  const double discountedStrike = strike_ * std::exp(-rate_ * tau);
  return std::max(discountedStrike - sMax_, 0.0);
}

PutTopBoundary::~PutTopBoundary() {}
