#include "RandomGenerator.h"

double RandomGenerator::Mean(unsigned long nbSim) {
  double sum = 0;
  for (unsigned long i = 0; i < nbSim; i++) {
    sum += Generate();
  }
  return sum / nbSim;
}