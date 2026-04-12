#include "SDE/Helper/ResetPathAtIndex.h"

#include "MonteCarlo/SinglePath.h"

#include <stdexcept>

namespace SDEHelper {

void ResetPathAtIndex(std::vector<SinglePath*>& paths,
                      size_t index,
                      double startTime,
                      double endTime,
                      size_t nbSteps,
                      double initialValue) {
  if (index >= paths.size()) {
    throw std::runtime_error("ResetPathAtIndex index out of bounds");
  }

  delete paths[index];
  paths[index] = new SinglePath(startTime, endTime, nbSteps);
  paths[index]->InsertValue(initialValue);
}

}  // namespace SDEHelper