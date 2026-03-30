#pragma once

#include "PDE/R2R1Function.h"

class BSVariance : public R2R1Function {
public:
  BSVariance(double sigma);
  double operator()(double x, double t) override;
  virtual ~BSVariance();

private:
  double sigma_;
};
