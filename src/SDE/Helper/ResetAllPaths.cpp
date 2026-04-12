#include "SDE/Helper/ResetAllPaths.h"

#include "SDE/Helper/ResetPathAtIndex.h"

namespace SDEHelper {

void ResetAllPaths(std::vector<SinglePath*>& paths,
                   size_t dimension,
                   double startTime,
                   double endTime,
                   size_t nbSteps,
                   double initialValue) {
  for (size_t d = 0; d < dimension; ++d) {
    ResetPathAtIndex(paths, d, startTime, endTime, nbSteps, initialValue);
  }
}

}  // namespace SDEHelper