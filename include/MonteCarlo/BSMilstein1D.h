#pragma once

#include "SDE/BlackScholes1D.h"

// Milstein scheme for one-dimensional Black-Scholes dynamics.
class BSMilstein1D : public BlackScholes1D {
public:
  BSMilstein1D(RandomGenerator* generator, double spot, double rate, double vol);

  // Simulates one asset path with Milstein second-order correction.
  void Simulate(double startTime, double endTime, size_t nbSteps) override;
};
