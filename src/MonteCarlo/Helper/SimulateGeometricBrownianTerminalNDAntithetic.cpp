#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalNDAntithetic.h"

#include "MonteCarlo/Helper/GeometricBrownianExactStep.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "MonteCarlo/SinglePath.h"
#include "SDE/BrownianND.h"

#include <cmath>
#include <stdexcept>

namespace MonteCarloHelper {

TerminalSpotsAntitheticPair SimulateGeometricBrownianTerminalNDAntithetic(
    RandomGenerator* generator,
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    double rate,
    std::vector<std::vector<double>>* loadingMatrix,
    double startTime,
    double endTime,
    size_t nbSteps) {
  ValidateTimeGrid("SimulateGeometricBrownianTerminalNDAntithetic", startTime,
                   endTime, nbSteps);
  if (spotPrices.size() != volatilities.size()) {
    throw std::runtime_error(
        "SimulateGeometricBrownianTerminalNDAntithetic requires matching spot and volatility sizes");
  }

  BrownianND brownian(generator, static_cast<int>(spotPrices.size()), loadingMatrix);
  brownian.Simulate(startTime, endTime, nbSteps);

  TerminalSpotsAntitheticPair result;
  result.direct = spotPrices;
  result.antithetic = spotPrices;

  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);

  for (size_t step = 0; step < nbSteps; ++step) {
    const double currentTime = startTime + static_cast<double>(step) * dt;
    const double nextTime = currentTime + dt;

    for (size_t i = 0; i < spotPrices.size(); ++i) {
      const SinglePath* path = brownian.GetPath(static_cast<int>(i));
      if (path == nullptr) {
        throw std::runtime_error(
            "SimulateGeometricBrownianTerminalNDAntithetic failed to retrieve a Brownian path");
      }

      const double increment = path->GetState(nextTime) - path->GetState(currentTime);
      const double shock = increment / sqrtDt;

      result.direct[i] = GeometricBrownianExactStep(result.direct[i], rate,
                                                    volatilities[i], dt, shock);
      result.antithetic[i] = GeometricBrownianExactStep(result.antithetic[i], rate,
                                                        volatilities[i], dt, -shock);
    }
  }

  return result;
}

}  // namespace MonteCarloHelper
