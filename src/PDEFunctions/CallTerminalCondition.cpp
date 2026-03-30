#include "PDEFunctions/CallTerminalCondition.h"

CallTerminalCondition::CallTerminalCondition(double strike)
    : VanillaTerminalCondition(strike) {}

double CallTerminalCondition::operator()(double x) {
  if (x > strike_) {
    return x - strike_;
  }
  return 0.0;
}
