#include "RandomGenerator.h"
#include <stdexcept>

double RandomGenerator::Mean(unsigned long nbSim) {
  if (nbSim == 0) {
    throw std::invalid_argument("RandomGenerator::Mean requires nbSim > 0");
  }

  double sum = 0;
  for (unsigned long i = 0; i < nbSim; i++) {
    sum += Generate();
  }
  return sum / nbSim;
}
