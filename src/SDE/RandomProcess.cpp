#include "SDE/RandomProcess.h"
#include <stdexcept>

RandomProcess::RandomProcess(RandomGenerator* generator, int dimension)
    : Generator_(generator), Dimension_(dimension) {
  if (generator == nullptr) {
    throw std::runtime_error("RandomProcess requires a valid random generator");
  }
  if (dimension <= 0) {
    throw std::runtime_error("RandomProcess requires a strictly positive dimension");
  }
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
