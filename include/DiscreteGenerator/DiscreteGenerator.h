#pragma once

#include "../RandomGenerator.h"

// DiscreteGenerator - Abstract intermediate class for discrete distributions
// Examples: Bernoulli, Binomial, Poisson, HeadTail, FiniteSet
// This class doesn't add new functionality, but categorizes discrete generators

class DiscreteGenerator : public RandomGenerator {
public:
  // Generate remains pure virtual - each discrete distribution implements it
  virtual double Generate() = 0;

  // Virtual destructor
  virtual ~DiscreteGenerator() {}
};
