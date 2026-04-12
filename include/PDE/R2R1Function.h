#pragma once

// Interface for scalar functions f: R x R -> R used in PDE coefficients.
class R2R1Function {
public:
  R2R1Function();

  // Evaluates f(x, t).
  virtual double operator()(double x, double t) = 0;

  virtual ~R2R1Function();
};
