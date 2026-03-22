#pragma once

#include "LinearCongruential.h"

class EcuyerCombined : public PseudoGenerator {
public:
  EcuyerCombined(double seed1, double seed2);

  double Generate() override;

  virtual ~EcuyerCombined() {}

private:
  LinearCongruential FirstGenerator;
  LinearCongruential SecondGenerator;
};
