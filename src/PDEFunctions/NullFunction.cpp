#include "PDEFunctions/NullFunction.h"

NullFunction::NullFunction() {}

double NullFunction::operator()(double x, double t) {
  (void)x;
  (void)t;
  return 0.0;
}

NullFunction::~NullFunction() {}
