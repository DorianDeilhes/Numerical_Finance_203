#pragma once

#include "RandomGenerator.h"

#include <cstddef>

class SinglePath;

namespace MonteCarloHelper {

void FillGeometricBrownianEulerPath(RandomGenerator* generator,
                                    SinglePath* path,
                                    double spot,
                                    double rate,
                                    double vol,
                                    double startTime,
                                    double endTime,
                                    size_t nbSteps);

}  // namespace MonteCarloHelper