#include "SDE/Brownian1D.h"
#include "ContinuousGenerator/Normal.h"
#include "UniformGenerator/UniformGenerator.h"
#include <cmath>
#include <stdexcept>

Brownian1D::Brownian1D(RandomGenerator* generator)
    : RandomProcess(generator, 1) {}

void Brownian1D::Simulate(double startTime, double endTime, size_t nbSteps) {
  delete Paths_[0];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[0]->InsertValue(0.0);

  UniformGenerator* uniform = dynamic_cast<UniformGenerator*>(Generator_);
  if (uniform == nullptr) {
    throw std::runtime_error("Brownian1D requires a UniformGenerator");
  }

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);

  double value = 0.0;
  for (size_t step = 0; step < nbSteps; ++step) {
    value += sqrtDt * standardNormal.Generate();
    Paths_[0]->InsertValue(value);
  }
}
