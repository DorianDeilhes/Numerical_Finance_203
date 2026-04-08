#pragma once

#include "SDE/RandomProcess.h"
#include <vector>

class BrownianND : public RandomProcess {
public:
  BrownianND(RandomGenerator* generator, int dimension,
             std::vector<std::vector<double>>* correlationMatrix);
  void Simulate(double startTime, double endTime, size_t nbSteps) override;

protected:
  std::vector<std::vector<double>>* CorrelationMatrix_;
};
