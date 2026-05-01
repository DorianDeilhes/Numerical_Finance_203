#include "MonteCarlo/Helper/SimulateGeometricBrownianSpotsAtStepIndicesND.h"

#include "MonteCarlo/Helper/GeometricBrownianExactStep.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "MonteCarlo/SinglePath.h"
#include "SDE/BrownianND.h"

#include <cmath>
#include <stdexcept>

namespace MonteCarloHelper {

std::vector<std::vector<double>> SimulateGeometricBrownianSpotsAtStepIndicesND(
    RandomGenerator* generator,
    const std::vector<double>& spot_prices,
    const std::vector<double>& volatilities,
    double rate,
    std::vector<std::vector<double>>* loading_matrix,
    double start_time,
    double end_time,
    size_t nb_steps,
    const std::vector<size_t>& step_indices) {
  ValidateTimeGrid("SimulateGeometricBrownianSpotsAtStepIndicesND", start_time, end_time,
                   nb_steps);
  if (spot_prices.size() != volatilities.size()) {
    throw std::runtime_error(
        "SimulateGeometricBrownianSpotsAtStepIndicesND requires matching spot and volatility sizes");
  }
  if (step_indices.empty()) {
    throw std::runtime_error("SimulateGeometricBrownianSpotsAtStepIndicesND requires non-empty step indices");
  }

  for (size_t i = 0; i < step_indices.size(); ++i) {
    const size_t step = step_indices[i];
    if (step > nb_steps) {
      throw std::runtime_error(
          "SimulateGeometricBrownianSpotsAtStepIndicesND step index outside [0, nb_steps]");
    }
    if (step == 0 && i != 0) {
      throw std::runtime_error(
          "SimulateGeometricBrownianSpotsAtStepIndicesND only accepts step 0 as the first index");
    }
    if (i > 0 && step_indices[i - 1] >= step) {
      throw std::runtime_error(
          "SimulateGeometricBrownianSpotsAtStepIndicesND step indices must be strictly increasing");
    }
  }

  BrownianND brownian(generator, static_cast<int>(spot_prices.size()), loading_matrix);
  brownian.Simulate(start_time, end_time, nb_steps);

  std::vector<double> current_spots = spot_prices;
  std::vector<std::vector<double>> recorded_spots(step_indices.size(),
                                                   std::vector<double>(spot_prices.size(), 0.0));

  const double dt = (end_time - start_time) / static_cast<double>(nb_steps);
  size_t record_cursor = 0;
  if (step_indices[record_cursor] == 0) {
    recorded_spots[record_cursor] = current_spots;
    ++record_cursor;
  }

  for (size_t step = 0; step < nb_steps; ++step) {
    const double current_time = start_time + static_cast<double>(step) * dt;
    const double next_time = current_time + dt;

    for (size_t i = 0; i < spot_prices.size(); ++i) {
      const SinglePath* path = brownian.GetPath(static_cast<int>(i));
      if (path == nullptr) {
        throw std::runtime_error(
            "SimulateGeometricBrownianSpotsAtStepIndicesND failed to retrieve a Brownian path");
      }
      const double increment = path->GetState(next_time) - path->GetState(current_time);
      current_spots[i] = GeometricBrownianExactStep(current_spots[i], rate, volatilities[i], dt,
                                                    increment / std::sqrt(dt));
    }

    // Record spot snapshots exactly at requested (1-based) simulation steps.
    const size_t one_based_step = step + 1;
    if (record_cursor < step_indices.size() && one_based_step == step_indices[record_cursor]) {
      recorded_spots[record_cursor] = current_spots;
      ++record_cursor;
    }
  }

  if (record_cursor != step_indices.size()) {
    throw std::runtime_error(
        "SimulateGeometricBrownianSpotsAtStepIndicesND failed to record all requested step indices");
  }

  return recorded_spots;
}

}  // namespace MonteCarloHelper
