#include "SDE/Heston.h"
#include "ContinuousGenerator/Normal.h"
#include "UniformGenerator/UniformGenerator.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

Heston::Heston(RandomGenerator* generator, double spot, double initVariance,
               double mu, double theta, double kappa, double sigma, double rho)
    : RandomProcess(generator, 2), Spot_(spot), InitVariance_(initVariance),
      Mu_(mu), Theta_(theta), Kappa_(kappa), Sigma_(sigma), Rho_(rho) {}

void Heston::Simulate(double startTime, double endTime, size_t nbSteps) {
  UniformGenerator* uniform = dynamic_cast<UniformGenerator*>(Generator_);
  if (uniform == nullptr) {
    throw std::runtime_error("Heston requires a UniformGenerator");
  }

  delete Paths_[0];
  delete Paths_[1];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[1] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[0]->InsertValue(Spot_);
  Paths_[1]->InsertValue(InitVariance_);

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);
  const double rhoPerp = std::sqrt(std::max(0.0, 1.0 - Rho_ * Rho_));

  double spot = Spot_;
  double variance = InitVariance_;

  for (size_t step = 0; step < nbSteps; ++step) {
    const double z1 = standardNormal.Generate();
    const double z2 = standardNormal.Generate();
    const double wS = z1;
    const double wV = Rho_ * z1 + rhoPerp * z2;

    variance = variance + Kappa_ * (Theta_ - variance) * dt +
               Sigma_ * std::sqrt(std::max(variance, 0.0)) * sqrtDt * wV;
    variance = std::max(variance, 0.0);

    spot = spot + Mu_ * spot * dt + std::sqrt(std::max(variance, 0.0)) * spot * sqrtDt * wS;

    Paths_[0]->InsertValue(spot);
    Paths_[1]->InsertValue(variance);
  }
}
