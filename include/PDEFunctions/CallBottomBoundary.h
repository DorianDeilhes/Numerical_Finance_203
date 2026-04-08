#pragma once

#include "PDE/R1R1Function.h"

/**
 * Lower boundary for a European call.
 * Uses a time-dependent truncation formula:
 *   V(t, S_min) = max(S_min - K e^{-r(T-t)}, 0)
 *
 * The rate and maturity default to 0.0 for backward compatibility.
 */
class CallBottomBoundary : public R1R1Function {
public:
  CallBottomBoundary(double sMin, double strike, double rate = 0.0,
                     double maturity = 0.0);
  double operator()(double t) override;
  virtual ~CallBottomBoundary();

private:
  double sMin_;
  double strike_;
  double rate_;
  double maturity_;
};
