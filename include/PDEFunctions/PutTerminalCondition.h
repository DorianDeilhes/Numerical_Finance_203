#pragma once

#include "PDEFunctions/VanillaTerminalCondition.h"

class PutTerminalCondition : public VanillaTerminalCondition {
public:
  PutTerminalCondition(double strike);
  double operator()(double x) override;
};
