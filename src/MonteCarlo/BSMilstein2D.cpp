#include "MonteCarlo/BSMilstein2D.h"
#include "MonteCarlo/Helper/FillGeometricBrownianMilsteinPath2D.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"

#include <stdexcept>

BSMilstein2D::BSMilstein2D(RandomGenerator* generator, double spot1,
                           double spot2, double rate1, double rate2,
                           double vol1, double vol2, double rho)
    : BlackScholes2D(generator, spot1, spot2, rate1, rate2, vol1, vol2, rho) {}

void BSMilstein2D::Simulate(double startTime, double endTime, size_t nbSteps) {
  MonteCarloHelper::ValidateTimeGrid("BSMilstein2D::Simulate", startTime, endTime, nbSteps);

  // Reset both asset paths before simulation.
  delete Paths_[0];
  delete Paths_[1];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[1] = new SinglePath(startTime, endTime, nbSteps);
  MonteCarloHelper::FillGeometricBrownianMilsteinPath2D(Generator_, Paths_[0], Paths_[1],
                                                         Spot1_, Spot2_, Rate1_, Rate2_,
                                                         Vol1_, Vol2_, Rho_, startTime,
                                                         endTime, nbSteps);
}
