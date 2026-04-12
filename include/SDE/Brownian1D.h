#pragma once

#include "SDE/RandomProcess.h"

// One-dimensional Brownian motion W_t.
class Brownian1D : public RandomProcess {
public:
  Brownian1D(RandomGenerator* generator);

  // Simulates W_t increments with dW = sqrt(dt) * Z.
  void Simulate(double startTime, double endTime, size_t nbSteps) override;
};
