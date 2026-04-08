#include "SDE/RandomProcess.h"

RandomProcess::RandomProcess(RandomGenerator* generator, int dimension)
    : Generator_(generator), Dimension_(dimension) {
  Paths_.resize(dimension, nullptr);
}

RandomProcess::~RandomProcess() {
  for (SinglePath* path : Paths_) {
    delete path;
  }
}

SinglePath* RandomProcess::GetPath(int dimension) const {
  if (dimension < 0 || dimension >= Dimension_) {
    return nullptr;
  }
  return Paths_[static_cast<size_t>(dimension)];
}
