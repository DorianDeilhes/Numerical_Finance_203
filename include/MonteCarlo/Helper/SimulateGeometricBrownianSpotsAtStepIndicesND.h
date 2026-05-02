#pragma once

#include "RandomGenerator.h"

#include <cstddef>
#include <vector>

namespace MonteCarloHelper {

// Simulate ND Black-Scholes paths and record only the requested exercise-date snapshots.
std::vector<std::vector<double>> SimulateGeometricBrownianSpotsAtStepIndicesND(
    RandomGenerator* generator,
    const std::vector<double>& spot_prices,
    const std::vector<double>& volatilities,
    double rate,
    const std::vector<double>& dividend_yields,
    std::vector<std::vector<double>>* loading_matrix,
    double start_time,
    double end_time,
    size_t nb_steps,
    const std::vector<size_t>& step_indices);

}  // namespace MonteCarloHelper
