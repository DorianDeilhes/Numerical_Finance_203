#pragma once

class R2R1Function {
public:
  R2R1Function();
  virtual double operator()(double x, double t) = 0;
  virtual ~R2R1Function();
};
