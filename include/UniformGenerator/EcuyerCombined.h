#pragma once

#include "LinearCongruential.h"

// Combined LCG generator (Ecuyer-style) for improved period/quality.
class EcuyerCombined : public PseudoGenerator {
public:
  // Initializes both internal LCG components.
  EcuyerCombined(double seed1, double seed2);

  // Returns one combined pseudo-uniform sample.
  double Generate() override;

  virtual ~EcuyerCombined() {}

private:
  LinearCongruential FirstGenerator;
  LinearCongruential SecondGenerator;
};
