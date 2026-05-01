#pragma once

#include <cstddef>
#include <string>

class UniformGenerator;

namespace PricingHelper {

void ValidateUniformGeneratorPointer(UniformGenerator* uniform_gen,
                                     const std::string& method_name);

void ValidateCountAtLeast(size_t value,
                          size_t minimum,
                          const std::string& method_name,
                          const std::string& parameter_name);

}  // namespace PricingHelper
