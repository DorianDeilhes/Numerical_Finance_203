#include "MonteCarlo/Helper/FillGeometricBrownianMilsteinPath2D.h"

#include "ContinuousGenerator/Normal.h"
#include "MonteCarlo/Helper/GeometricBrownianMilsteinStep.h"
#include "MonteCarlo/Helper/GetUniformGeneratorOrThrow.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "MonteCarlo/SinglePath.h"

#include <cmath>
#include <stdexcept>

namespace MonteCarloHelper {

void FillGeometricBrownianMilsteinPath2D(RandomGenerator* generator,
                                         SinglePath* path1,
                                         SinglePath* path2,
                                         double spot1,
                                         double spot2,
                                         double rate1,
                                         double rate2,
                                         double vol1,
                                         double vol2,
                                         double rho,
                                         double startTime,
                                         double endTime,
                                         size_t nbSteps) {
  if (path1 == nullptr || path2 == nullptr) {
    throw std::runtime_error("FillGeometricBrownianMilsteinPath2D requires valid paths");
  }
  ValidateTimeGrid("FillGeometricBrownianMilsteinPath2D", startTime, endTime, nbSteps);

  UniformGenerator* uniform =
      GetUniformGeneratorOrThrow(generator, "FillGeometricBrownianMilsteinPath2D");

  path1->InsertValue(spot1);
  path2->InsertValue(spot2);

  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double rhoPerp = std::sqrt(std::max(0.0, 1.0 - rho * rho));

  double currentSpot1 = spot1;
  double currentSpot2 = spot2;
  for (size_t step = 0; step < nbSteps; ++step) {
    const double z1 = standardNormal.Generate();
    const double z2 = standardNormal.Generate();
    const double w1 = z1;
    const double w2 = rho * z1 + rhoPerp * z2;

    currentSpot1 = GeometricBrownianMilsteinStep(currentSpot1, rate1, vol1, dt, w1);
    currentSpot2 = GeometricBrownianMilsteinStep(currentSpot2, rate2, vol2, dt, w2);
    path1->InsertValue(currentSpot1);
    path2->InsertValue(currentSpot2);
  }
}

}  // namespace MonteCarloHelper