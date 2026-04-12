#pragma once

#include "RandomGenerator.h"

#include <cstddef>

class SinglePath;

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
                                         size_t nbSteps);

}  // namespace MonteCarloHelper