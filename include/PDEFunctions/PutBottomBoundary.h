#pragma once

#include "PDE/R1R1Function.h"

class PutBottomBoundary : public R1R1Function {
public:
  PutBottomBoundary(double sMin, double strike);
  double operator()(double t) override;
  virtual ~PutBottomBoundary();

private:
  double sMin_;
  double strike_;
};
