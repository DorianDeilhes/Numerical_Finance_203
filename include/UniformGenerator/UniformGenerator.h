#pragma once

#include "../RandomGenerator.h"

// Base interface for generators targeting U(0,1)-type outputs.
class UniformGenerator : public RandomGenerator {
public:
  // Generates one uniform-like sample used by higher-level generators.
  virtual double Generate() = 0;

  virtual ~UniformGenerator() {}
};
