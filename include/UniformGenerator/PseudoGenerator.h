#pragma once

#include "UniformGenerator.h"

// Base class for pseudo-random uniform generators with explicit seed state.
class PseudoGenerator : public UniformGenerator {
public:
  // Initializes the generator state from an external seed value.
  PseudoGenerator(double seed);

  virtual double Generate() = 0;

  // Returns the current internal seed/state value.
  double getSeed() const;

  virtual ~PseudoGenerator() {}

protected:
  double Seed;
};
