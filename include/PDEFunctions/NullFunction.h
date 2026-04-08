#pragma once
#include "PDE/R2R1Function.h"

class NullFunction : public R2R1Function {
public:
  NullFunction();
  virtual double operator()(double x, double t);
  virtual ~NullFunction();
};
