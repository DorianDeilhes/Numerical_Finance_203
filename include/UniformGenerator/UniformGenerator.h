#pragma once

#include "../RandomGenerator.h"

class UniformGenerator : public RandomGenerator {
public:
  virtual double Generate() = 0;

  virtual ~UniformGenerator() {}
};
