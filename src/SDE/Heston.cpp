#include "SDE/Heston.h"
#include "MonteCarlo/Helper/GetUniformGeneratorOrThrow.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "SDE/Helper/ResetPathAtIndex.h"
#include "ContinuousGenerator/Normal.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

Heston::Heston(RandomGenerator* generator, double spot, double initVariance,
               double mu, double theta, double kappa, double sigma, double rho)
    : RandomProcess(generator, 2), Spot_(spot), InitVariance_(initVariance),
      Mu_(mu), Theta_(theta), Kappa_(kappa), Sigma_(sigma), Rho_(rho) {
  if (!(spot > 0.0)) {
    throw std::runtime_error("Heston requires a strictly positive spot");
  }
  if (initVariance < 0.0) {
    throw std::runtime_error("Heston requires a non-negative initial variance");
  }
  if (theta < 0.0 || kappa < 0.0 || sigma < 0.0) {
    throw std::runtime_error("Heston requires non-negative theta, kappa and sigma");
  }
  if (rho < -1.0 || rho > 1.0) {
    throw std::runtime_error("Heston requires rho in [-1, 1]");
  }
}


void Heston::Simulate(double startTime, double endTime, size_t nbSteps) {
  MonteCarloHelper::ValidateTimeGrid("Heston::Simulate", startTime, endTime, nbSteps);

  UniformGenerator* uniform =
      MonteCarloHelper::GetUniformGeneratorOrThrow(Generator_, "Heston::Simulate");

  // Reset state paths: spot and instantaneous variance.
  SDEHelper::ResetPathAtIndex(Paths_, 0, startTime, endTime, nbSteps, Spot_);
  SDEHelper::ResetPathAtIndex(Paths_, 1, startTime, endTime, nbSteps, InitVariance_);

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);
  const double rhoPerp = std::sqrt(std::max(0.0, 1.0 - Rho_ * Rho_));

  double spot = Spot_;
  double variance = InitVariance_;

  for (size_t step = 0; step < nbSteps; ++step) {
    const double z1 = standardNormal.Generate();
    const double z2 = standardNormal.Generate();
    // Correlated Brownian shocks for spot and variance factors.
    const double wS = z1;
    const double wV = Rho_ * z1 + rhoPerp * z2;

    // Full truncation on variance preserves non-negativity in the diffusion term.
    variance = variance + Kappa_ * (Theta_ - variance) * dt +
               Sigma_ * std::sqrt(std::max(variance, 0.0)) * sqrtDt * wV;
    variance = std::max(variance, 0.0);

    spot = spot + Mu_ * spot * dt + std::sqrt(std::max(variance, 0.0)) * spot * sqrtDt * wS;

    Paths_[0]->InsertValue(spot);
    Paths_[1]->InsertValue(variance);
  }
}
