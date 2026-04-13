#include "UniformGenerator/Helper/ValidateQuasiRandomConfig.h"

#include <cmath>
#include <stdexcept>

namespace UniformGeneratorHelper {

void ValidateQuasiRandomConfig(size_t dimension, double shiftSeed) {
  if (dimension == 0) {
    throw std::runtime_error("Quasi-random generators require dimension > 0");
  }
  if (dimension > 4096) {
    throw std::runtime_error("Quasi-random generators require dimension <= 4096");
  }
  if (!std::isfinite(shiftSeed)) {
    throw std::runtime_error("Quasi-random generators require a finite shift seed");
  }
}

}  // namespace UniformGeneratorHelper