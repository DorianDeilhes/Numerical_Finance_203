#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalND.h"

#include "MonteCarlo/Helper/GeometricBrownianExactStep.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "MonteCarlo/SinglePath.h"
#include "SDE/BrownianND.h"

#include <cmath>
#include <stdexcept>

namespace MonteCarloHelper {

std::vector<double> SimulateGeometricBrownianTerminalND(
    RandomGenerator* generator,
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    double rate,
    const std::vector<double>& dividendYields,
    std::vector<std::vector<double>>* loadingMatrix,
    double startTime,
    double endTime,
    size_t nbSteps) {
  ValidateTimeGrid("SimulateGeometricBrownianTerminalND", startTime, endTime, nbSteps);
  if (spotPrices.size() != volatilities.size()) {
    throw std::runtime_error("SimulateGeometricBrownianTerminalND requires matching spot and volatility sizes");
  }
  if (dividendYields.size() != spotPrices.size()) {
    throw std::runtime_error("SimulateGeometricBrownianTerminalND requires matching dividend yield and spot sizes");
  }

  BrownianND brownian(generator, static_cast<int>(spotPrices.size()), loadingMatrix);
  brownian.Simulate(startTime, endTime, nbSteps);

  std::vector<double> terminalSpots = spotPrices;
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  for (size_t step = 0; step < nbSteps; ++step) {
    const double currentTime = startTime + static_cast<double>(step) * dt;
    const double nextTime = currentTime + dt;
    for (size_t i = 0; i < spotPrices.size(); ++i) {
      const SinglePath* path = brownian.GetPath(static_cast<int>(i));
      if (path == nullptr) {
        throw std::runtime_error("SimulateGeometricBrownianTerminalND failed to retrieve a Brownian path");
      }
      const double increment = path->GetState(nextTime) - path->GetState(currentTime);
      terminalSpots[i] = GeometricBrownianExactStep(terminalSpots[i], rate, volatilities[i], dt,
                                                    increment / std::sqrt(dt), dividendYields[i]);
    }
  }

  return terminalSpots;
}

}  // namespace MonteCarloHelper
