#include "UniformGenerator/LinearCongruential.h"
#include <cmath>

LinearCongruential::LinearCongruential(double seed, double multiplier,
                                       double increment, double modulus)
    : PseudoGenerator(seed), Multiplier(multiplier), Increment(increment),
      Modulus(modulus) {}

double LinearCongruential::Generate() {

  Seed = fmod(Multiplier * Seed + Increment, Modulus);

  return Seed / Modulus;
}