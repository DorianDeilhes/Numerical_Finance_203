#pragma once

#include "PDE/R2R1Function.h"

class BSTrend : public R2R1Function {
public:
  BSTrend(double rate);
  double operator()(double x, double t) override;
  virtual ~BSTrend();

private:
  double rate_;
};
