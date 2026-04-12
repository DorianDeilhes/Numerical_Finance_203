#pragma once

#include "RandomGenerator.h"

#include <string>

class UniformGenerator;

namespace MonteCarloHelper {

UniformGenerator* GetUniformGeneratorOrThrow(RandomGenerator* generator,
                                             const std::string& functionName);

}  // namespace MonteCarloHelper