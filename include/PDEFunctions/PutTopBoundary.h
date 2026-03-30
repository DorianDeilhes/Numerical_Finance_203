#pragma once

#include "PDE/R1R1Function.h"

class PutTopBoundary : public R1R1Function {
public:
  PutTopBoundary(double sMax, double strike);
  double operator()(double t) override;
  virtual ~PutTopBoundary();

private:
  double sMax_;
  double strike_;
};
