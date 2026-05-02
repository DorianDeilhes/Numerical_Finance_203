#pragma once

#include "RandomGenerator.h"

#include <cstddef>
#include <vector>

namespace MonteCarloHelper {

struct ExerciseSpotsAntitheticPair {
  std::vector<std::vector<double>> direct;
  std::vector<std::vector<double>> antithetic;
};

// Simulate direct and antithetic ND Black-Scholes paths, recording only requested snapshots.
ExerciseSpotsAntitheticPair SimulateGeometricBrownianSpotsAtStepIndicesNDAntithetic(
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
