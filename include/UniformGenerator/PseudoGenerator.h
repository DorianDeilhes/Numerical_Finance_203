#pragma once

#include "UniformGenerator.h"

class PseudoGenerator : public UniformGenerator {
public:
  PseudoGenerator(double seed);

  virtual double Generate() = 0;

  double getSeed() const;

  virtual ~PseudoGenerator() {}

protected:
  double Seed;
};
