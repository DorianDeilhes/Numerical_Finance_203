#include "MonteCarlo/Helper/BasketCallPayoff.h"

#include <algorithm>
#include <stdexcept>

namespace MonteCarloHelper {

double BasketCallPayoff(const std::vector<double>& terminalSpots,
                        const std::vector<double>& weights,
                        double strike) {
  if (terminalSpots.size() != weights.size()) {
    throw std::runtime_error("BasketCallPayoff requires matching spot and weight sizes");
  }

  double basketValue = 0.0;
  for (size_t i = 0; i < terminalSpots.size(); ++i) {
    basketValue += weights[i] * terminalSpots[i];
  }
  return std::max(basketValue - strike, 0.0);
}

}  // namespace MonteCarloHelper