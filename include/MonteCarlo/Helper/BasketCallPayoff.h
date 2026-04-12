#pragma once

#include <vector>

namespace MonteCarloHelper {

double BasketCallPayoff(const std::vector<double>& terminalSpots,
                        const std::vector<double>& weights,
                        double strike);

}  // namespace MonteCarloHelper