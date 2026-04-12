#include "MonteCarlo/Helper/GetUniformGeneratorOrThrow.h"

#include "UniformGenerator/UniformGenerator.h"

#include <stdexcept>

namespace MonteCarloHelper {

UniformGenerator* GetUniformGeneratorOrThrow(RandomGenerator* generator,
                                             const std::string& functionName) {
  UniformGenerator* uniform = dynamic_cast<UniformGenerator*>(generator);
  if (uniform == nullptr) {
    throw std::runtime_error(functionName + " requires generator to be a UniformGenerator");
  }
  return uniform;
}

}  // namespace MonteCarloHelper