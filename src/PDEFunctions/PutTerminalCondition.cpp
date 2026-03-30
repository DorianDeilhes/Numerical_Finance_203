#include "PDEFunctions/PutTerminalCondition.h"

PutTerminalCondition::PutTerminalCondition(double strike)
    : VanillaTerminalCondition(strike) {}

double PutTerminalCondition::operator()(double x) {
  if (x < strike_) {
    return strike_ - x;
  }
  return 0.0;
}
