#pragma once

#include "PDE/R1R1Function.h"

class CallBottomBoundary : public R1R1Function {
public:
  CallBottomBoundary(double sMin, double strike);
  double operator()(double t) override;
  virtual ~CallBottomBoundary();

private:
  double sMin_;
  double strike_;
};
