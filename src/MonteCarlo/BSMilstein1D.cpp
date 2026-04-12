#include "MonteCarlo/BSMilstein1D.h"
#include "MonteCarlo/Helper/FillGeometricBrownianMilsteinPath.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"

#include <stdexcept>

BSMilstein1D::BSMilstein1D(RandomGenerator* generator, double spot, double rate,
                           double vol)
    : BlackScholes1D(generator, spot, rate, vol) {}

void BSMilstein1D::Simulate(double startTime, double endTime, size_t nbSteps) {
  MonteCarloHelper::ValidateTimeGrid("BSMilstein1D::Simulate", startTime, endTime, nbSteps);

  // Reset path for one-dimensional asset trajectory.
  delete Paths_[0];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  MonteCarloHelper::FillGeometricBrownianMilsteinPath(Generator_, Paths_[0], Spot_, Rate_, Vol_,
                                                       startTime, endTime, nbSteps);
}
