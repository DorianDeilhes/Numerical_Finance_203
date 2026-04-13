#pragma once

#include <cstddef>

namespace UniformGeneratorHelper {

// Validates constructor inputs for quasi-random generators.
void ValidateQuasiRandomConfig(size_t dimension, double shiftSeed);

}  // namespace UniformGeneratorHelper
