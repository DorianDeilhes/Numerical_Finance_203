#pragma once

// Base interface for scalar random-variable generators.
class RandomGenerator {
public:
  // Generates one sample from the implemented distribution.
  virtual double Generate() = 0;

  // Returns the empirical mean over nbSim independent calls to Generate().
  double Mean(unsigned long nbSim);

  virtual ~RandomGenerator() {}
};
