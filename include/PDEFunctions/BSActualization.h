#pragma once

#include "PDE/R2R1Function.h"

class BSActualization : public R2R1Function {
public:
  BSActualization(double rate);
  double operator()(double x, double t) override;
  virtual ~BSActualization();

private:
  double rate_;
};
