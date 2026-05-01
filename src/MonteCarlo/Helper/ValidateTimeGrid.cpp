#include "MonteCarlo/Helper/ValidateTimeGrid.h"

#include <cmath>
#include <stdexcept>

namespace MonteCarloHelper {

void ValidateTimeGrid(const std::string& functionName,
                      double startTime,
                      double endTime,
                      size_t nbSteps) {
  if (!std::isfinite(startTime) || !std::isfinite(endTime)) {
    throw std::runtime_error(functionName + " requires finite startTime and endTime");
  }
  if (!(endTime > startTime)) {
    throw std::runtime_error(functionName + " requires endTime to be strictly greater than startTime");
  }
  if (nbSteps == 0) {
    throw std::runtime_error(functionName + " requires a strictly positive number of time steps");
  }
}

}  // namespace MonteCarloHelper
