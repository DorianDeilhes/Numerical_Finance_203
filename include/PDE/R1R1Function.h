#pragma once

// Interface for scalar functions f: R -> R used in boundaries/payoffs.
class R1R1Function {
public:
  R1R1Function();

  // Evaluates f(x).
  virtual double operator()(double x) = 0;

  virtual ~R1R1Function();
};
