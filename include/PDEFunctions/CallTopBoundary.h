#pragma once

#include "PDE/R1R1Function.h"

/**
 * Upper boundary for a European call.
 * Uses the time-dependent asymptotic form:
 *   V(t, S_max) = max(S_max - K e^{-r(T-t)}, 0)
 *
 * The rate and maturity default to 0.0 for backward compatibility.
 */
class CallTopBoundary : public R1R1Function {
public:
  CallTopBoundary(double sMax, double strike, double rate = 0.0,
                  double maturity = 0.0);
  double operator()(double t) override;
  virtual ~CallTopBoundary();

private:
  double sMax_;
  double strike_;
  double rate_;
  double maturity_;
};
