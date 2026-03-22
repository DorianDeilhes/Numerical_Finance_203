#pragma once

#include "PseudoGenerator.h"

class LinearCongruential : public PseudoGenerator {
public:
  LinearCongruential(double seed, double multiplier, double increment,
                     double modulus);

  double Generate() override;

  virtual ~LinearCongruential() {}

private:
  double Multiplier;
  double Increment;
  double Modulus;
};
