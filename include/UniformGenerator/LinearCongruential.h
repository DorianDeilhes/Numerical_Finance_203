#pragma once

#include "PseudoGenerator.h"

// Linear congruential pseudo-random generator.
class LinearCongruential : public PseudoGenerator {
public:
  // Builds an LCG with parameters (multiplier, increment, modulus).
  LinearCongruential(double seed, double multiplier, double increment,
                     double modulus);

  // Advances the recurrence and returns one pseudo-uniform sample.
  double Generate() override;

  virtual ~LinearCongruential() {}

private:
  double Multiplier;
  double Increment;
  double Modulus;
};
