#pragma once

#include "../RandomGenerator.h"

// ContinuousGenerator - Abstract intermediate class for continuous
// distributions Examples: Exponential, Normal This class doesn't add new
// functionality, but categorizes continuous generators

class ContinuousGenerator : public RandomGenerator {
public:
  // Generate remains pure virtual - each continuous distribution implements it
  virtual double Generate() = 0;

  // Virtual destructor
  virtual ~ContinuousGenerator() {}
};
