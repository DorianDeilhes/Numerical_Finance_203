#include "MonteCarlo/BSMilstein2D.h"
#include "ContinuousGenerator/Normal.h"
#include "UniformGenerator/UniformGenerator.h"
#include <cmath>
#include <stdexcept>

BSMilstein2D::BSMilstein2D(RandomGenerator* generator, double spot1,
                           double spot2, double rate1, double rate2,
                           double vol1, double vol2, double rho)
    : BlackScholes2D(generator, spot1, spot2, rate1, rate2, vol1, vol2, rho) {}

void BSMilstein2D::Simulate(double startTime, double endTime, size_t nbSteps) {
  UniformGenerator* uniform = dynamic_cast<UniformGenerator*>(Generator_);
  if (uniform == nullptr) {
    throw std::runtime_error("BSMilstein2D requires a UniformGenerator");
  }

  delete Paths_[0];
  delete Paths_[1];
  Paths_[0] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[1] = new SinglePath(startTime, endTime, nbSteps);
  Paths_[0]->InsertValue(Spot1_);
  Paths_[1]->InsertValue(Spot2_);

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);
  const double rho = Rho_;
  const double rhoPerp = std::sqrt(std::max(0.0, 1.0 - rho * rho));

  double s1 = Spot1_;
  double s2 = Spot2_;
  for (size_t step = 0; step < nbSteps; ++step) {
    const double z1 = standardNormal.Generate();
    const double z2 = standardNormal.Generate();
    const double w1 = z1;
    const double w2 = rho * z1 + rhoPerp * z2;

    s1 = s1 + Rate1_ * s1 * dt + Vol1_ * s1 * sqrtDt * w1 +
         0.5 * Vol1_ * Vol1_ * s1 * (w1 * w1 - 1.0) * dt;
    s2 = s2 + Rate2_ * s2 * dt + Vol2_ * s2 * sqrtDt * w2 +
         0.5 * Vol2_ * Vol2_ * s2 * (w2 * w2 - 1.0) * dt;

    Paths_[0]->InsertValue(s1);
    Paths_[1]->InsertValue(s2);
  }
}
