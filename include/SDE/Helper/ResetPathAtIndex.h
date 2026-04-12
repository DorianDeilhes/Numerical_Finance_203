#pragma once

#include <cstddef>
#include <vector>

class SinglePath;

namespace SDEHelper {

void ResetPathAtIndex(std::vector<SinglePath*>& paths,
                      size_t index,
                      double startTime,
                      double endTime,
                      size_t nbSteps,
                      double initialValue);

}  // namespace SDEHelper