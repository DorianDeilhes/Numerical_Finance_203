#include "Pricing/Helper/ValidatePricingMethodInputs.h"

#include <stdexcept>

namespace PricingHelper {

void ValidateUniformGeneratorPointer(UniformGenerator* uniform_gen,
                                     const std::string& method_name) {
  if (uniform_gen == nullptr) {
    throw std::runtime_error(method_name + " requires a non-null uniform generator");
  }
}

void ValidateCountAtLeast(size_t value,
                          size_t minimum,
                          const std::string& method_name,
                          const std::string& parameter_name) {
  if (value < minimum) {
    throw std::runtime_error(method_name + " requires " + parameter_name +
                             " >= " + std::to_string(minimum));
  }
}

}  // namespace PricingHelper
