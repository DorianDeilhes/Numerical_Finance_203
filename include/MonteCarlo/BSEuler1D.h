#pragma once

#include "SDE/BlackScholes1D.h"

// Euler scheme for one-dimensional Black-Scholes dynamics.
class BSEuler1D : public BlackScholes1D {
public:
  BSEuler1D(RandomGenerator* generator, double spot, double rate, double vol);

  // Simulates one asset path with Euler-Maruyama discretization.
  void Simulate(double startTime, double endTime, size_t nbSteps) override;
};
