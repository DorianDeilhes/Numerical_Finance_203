#include "PDEFunctions/BSVariance.h"

BSVariance::BSVariance(double sigma) : sigma_(sigma) {}

double BSVariance::operator()(double x, double t) {
  (void)t;
  return 0.5 * sigma_ * sigma_ * x * x;
}

BSVariance::~BSVariance() {}
