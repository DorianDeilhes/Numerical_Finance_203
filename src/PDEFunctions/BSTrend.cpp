#include "PDEFunctions/BSTrend.h"

BSTrend::BSTrend(double rate) : rate_(rate) {}

double BSTrend::operator()(double x, double t) {
  (void)t;
  return rate_ * x;
}

BSTrend::~BSTrend() {}
