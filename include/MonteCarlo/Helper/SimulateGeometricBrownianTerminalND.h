#pragma once

#include "RandomGenerator.h"

#include <cstddef>
#include <vector>

namespace MonteCarloHelper {

std::vector<double> SimulateGeometricBrownianTerminalND(
    RandomGenerator* generator,
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    double rate,
    std::vector<std::vector<double>>* loadingMatrix,
    double startTime,
    double endTime,
    size_t nbSteps);

}  // namespace MonteCarloHelper