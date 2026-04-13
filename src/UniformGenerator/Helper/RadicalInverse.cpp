#include "UniformGenerator/Helper/RadicalInverse.h"

#include <stdexcept>

namespace UniformGeneratorHelper {

double RadicalInverse(unsigned int base, size_t n) {
  if (base < 2U) {
    throw std::runtime_error("RadicalInverse requires base >= 2");
  }

  const double invBase = 1.0 / static_cast<double>(base);
  double factor = invBase;
  double result = 0.0;

  size_t value = n;
  while (value > 0) {
    const unsigned int digit = static_cast<unsigned int>(value % base);
    result += static_cast<double>(digit) * factor;
    value /= base;
    factor *= invBase;
  }

  return result;
}

}  // namespace UniformGeneratorHelper
