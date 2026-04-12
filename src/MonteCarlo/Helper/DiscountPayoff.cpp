#include "MonteCarlo/Helper/DiscountPayoff.h"

#include <cmath>

namespace MonteCarloHelper {

double DiscountPayoff(double payoff, double rate, double maturity) {
  return payoff * std::exp(-rate * maturity);
}

}  // namespace MonteCarloHelper