#pragma once

class R1R1Function {
public:
  R1R1Function();
  virtual double operator()(double x) = 0;
  virtual ~R1R1Function();
};
