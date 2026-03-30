#pragma once

#include "PDEFunctions/VanillaTerminalCondition.h"

class CallTerminalCondition : public VanillaTerminalCondition {
public:
  CallTerminalCondition(double strike);
  double operator()(double x) override;
};
