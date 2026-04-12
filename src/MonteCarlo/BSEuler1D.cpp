#include "MonteCarlo/BSEuler1D.h"
#include "MonteCarlo/Helper/FillGeometricBrownianEulerPath.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"

#include <stdexcept>

BSEuler1D::BSEuler1D(RandomGenerator* generator, double spot, double rate,
                     double vol)
    : BlackScholes1D(generator, spot, rate, vol) {}

void BSEuler1D::Simulate(double startTime, double endTime, size_t nbSteps) {
  MonteCarloHelper::ValidateTimeGrid("BSEuler1D::Simulate", startTime, endTime, nbSteps);

  // Reset path for one-dimensional asset trajectory.
  delete Paths_[0];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  MonteCarloHelper::FillGeometricBrownianEulerPath(Generator_, Paths_[0], Spot_, Rate_, Vol_,
                                                    startTime, endTime, nbSteps);
}
