#pragma once

#include <cstddef>
#include <string>

namespace MonteCarloHelper {

void ValidateTimeGrid(const std::string& functionName,
                      double startTime,
                      double endTime,
                      size_t nbSteps);

}  // namespace MonteCarloHelper