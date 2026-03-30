#pragma once
#include "PDE/R1R1Function.h"

class VanillaTerminalCondition : public R1R1Function {
public:
  VanillaTerminalCondition (double strike);
  virtual ~VanillaTerminalCondition();
protected:
  double strike_;
};

