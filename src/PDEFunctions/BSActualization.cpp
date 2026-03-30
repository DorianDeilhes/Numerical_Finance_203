#include "PDEFunctions/BSActualization.h"

BSActualization::BSActualization(double rate) : rate_(rate) {}

double BSActualization::operator()(double x, double t) {
  (void)x;
  (void)t;
  return rate_;
}

BSActualization::~BSActualization() {}
