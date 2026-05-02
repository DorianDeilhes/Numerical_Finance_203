#pragma once

#include "RandomGenerator.h"

#include <cstddef>
#include <vector>

namespace MonteCarloHelper {

struct TerminalSpotsAntitheticPair {
  std::vector<double> direct;
  std::vector<double> antithetic;
};

// Returns two terminal spot vectors built from the same Brownian increments:
// one direct path and one antithetic path using opposite shocks.
TerminalSpotsAntitheticPair SimulateGeometricBrownianTerminalNDAntithetic(
    RandomGenerator* generator,
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    double rate,
    const std::vector<double>& dividendYields,
    std::vector<std::vector<double>>* loadingMatrix,
    double startTime,
    double endTime,
    size_t nbSteps);

}  // namespace MonteCarloHelper
