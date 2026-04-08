#pragma once

#include "PDE/R1R1Function.h"

/**
 * Lower boundary for a European put.
 * Uses the time-dependent asymptotic form:
 *   V(t, S_min) = max(K e^{-r(T-t)} - S_min, 0)
 *
 * The rate and maturity default to 0.0 for backward compatibility.
 */
class PutBottomBoundary : public R1R1Function {
public:
  PutBottomBoundary(double sMin, double strike, double rate = 0.0,
                    double maturity = 0.0);
  double operator()(double t) override;
  virtual ~PutBottomBoundary();

private:
  double sMin_;
  double strike_;
  double rate_;
  double maturity_;
};
