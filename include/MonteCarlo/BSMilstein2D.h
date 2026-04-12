#pragma once

#include "SDE/BlackScholes2D.h"

// Milstein scheme for two correlated Black-Scholes assets.
class BSMilstein2D : public BlackScholes2D {
public:
  BSMilstein2D(RandomGenerator* generator, double spot1, double spot2,
               double rate1, double rate2, double vol1, double vol2,
               double rho);

  // Simulates two correlated asset paths with Milstein discretization.
  void Simulate(double startTime, double endTime, size_t nbSteps) override;
};
