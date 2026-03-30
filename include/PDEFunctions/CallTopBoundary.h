#pragma once

#include "PDE/R1R1Function.h"

class CallTopBoundary : public R1R1Function {
public:
  CallTopBoundary(double sMax, double strike);
  double operator()(double t) override;
  virtual ~CallTopBoundary();

private:
  double sMax_;
  double strike_;
};
