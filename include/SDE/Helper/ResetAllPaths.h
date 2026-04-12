#pragma once

#include <cstddef>
#include <vector>

class SinglePath;

namespace SDEHelper {

void ResetAllPaths(std::vector<SinglePath*>& paths,
                   size_t dimension,
                   double startTime,
                   double endTime,
                   size_t nbSteps,
                   double initialValue);

}  // namespace SDEHelper