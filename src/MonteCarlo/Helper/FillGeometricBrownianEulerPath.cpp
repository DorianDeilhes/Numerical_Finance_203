#include "MonteCarlo/Helper/FillGeometricBrownianEulerPath.h"

#include "ContinuousGenerator/Normal.h"
#include "MonteCarlo/Helper/GeometricBrownianEulerStep.h"
#include "MonteCarlo/Helper/GetUniformGeneratorOrThrow.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "MonteCarlo/SinglePath.h"

#include <stdexcept>

namespace MonteCarloHelper {

void FillGeometricBrownianEulerPath(RandomGenerator* generator,
                                    SinglePath* path,
                                    double spot,
                                    double rate,
                                    double vol,
                                    double startTime,
                                    double endTime,
                                    size_t nbSteps) {
  if (path == nullptr) {
    throw std::runtime_error("FillGeometricBrownianEulerPath requires a valid path");
  }
  ValidateTimeGrid("FillGeometricBrownianEulerPath", startTime, endTime, nbSteps);

  UniformGenerator* uniform =
      GetUniformGeneratorOrThrow(generator, "FillGeometricBrownianEulerPath");

  path->InsertValue(spot);
  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);
  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);

  double currentSpot = spot;
  for (size_t step = 0; step < nbSteps; ++step) {
    currentSpot = GeometricBrownianEulerStep(currentSpot, rate, vol, dt,
                                             standardNormal.Generate());
    path->InsertValue(currentSpot);
  }
}

}  // namespace MonteCarloHelper