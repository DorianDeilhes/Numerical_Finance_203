#include "MonteCarlo/BSEuler1D.h"
#include "ContinuousGenerator/Normal.h"
#include "UniformGenerator/UniformGenerator.h"
#include <cmath>
#include <stdexcept>

BSEuler1D::BSEuler1D(RandomGenerator* generator, double spot, double rate,
                     double vol)
    : BlackScholes1D(generator, spot, rate, vol) {}

void BSEuler1D::Simulate(double startTime, double endTime, size_t nbSteps) {
  delete Paths_[0];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[0]->InsertValue(Spot_);

  UniformGenerator* uniform = dynamic_cast<UniformGenerator*>(Generator_);
  if (uniform == nullptr) {
    throw std::runtime_error("BSEuler1D requires a UniformGenerator");
  }

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);

  double spot = Spot_;
  for (size_t step = 0; step < nbSteps; ++step) {
    const double z = standardNormal.Generate();
    spot = spot + Rate_ * spot * dt + Vol_ * spot * sqrtDt * z;
    Paths_[0]->InsertValue(spot);
  }
}
