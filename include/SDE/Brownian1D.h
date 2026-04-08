#pragma once

#include "SDE/RandomProcess.h"

class Brownian1D : public RandomProcess {
public:
  Brownian1D(RandomGenerator* generator);
  void Simulate(double startTime, double endTime, size_t nbSteps) override;
};
