#pragma once

#include "PDE/R1R1Function.h"

/**
 * Upper boundary for a European put.
 * Uses a time-dependent truncation formula:
 *   V(t, S_max) = max(K e^{-r(T-t)} - S_max, 0)
 *
 * The rate and maturity default to 0.0 for backward compatibility.
 */
class PutTopBoundary : public R1R1Function {
public:
  PutTopBoundary(double sMax, double strike, double rate = 0.0,
                 double maturity = 0.0);
  double operator()(double t) override;
  virtual ~PutTopBoundary();

private:
  double sMax_;
  double strike_;
  double rate_;
  double maturity_;
};
