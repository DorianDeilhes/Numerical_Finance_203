#include "SDE/Brownian1D.h"
#include "MonteCarlo/Helper/GetUniformGeneratorOrThrow.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "SDE/Helper/ResetPathAtIndex.h"
#include "ContinuousGenerator/Normal.h"
#include <cmath>
#include <stdexcept>

Brownian1D::Brownian1D(RandomGenerator* generator)
    : RandomProcess(generator, 1) {}

void Brownian1D::Simulate(double startTime, double endTime, size_t nbSteps) {
  MonteCarloHelper::ValidateTimeGrid("Brownian1D::Simulate", startTime, endTime, nbSteps);

  // Reset path storage for a fresh simulation run.
  SDEHelper::ResetPathAtIndex(Paths_, 0, startTime, endTime, nbSteps, 0.0);

  UniformGenerator* uniform =
      MonteCarloHelper::GetUniformGeneratorOrThrow(Generator_, "Brownian1D::Simulate");

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);

  // Euler increment for W_t: dW = sqrt(dt) * Z.
  double value = 0.0;
  for (size_t step = 0; step < nbSteps; ++step) {
    value += sqrtDt * standardNormal.Generate();
    Paths_[0]->InsertValue(value);
  }
}
