#pragma once

#include "SDE/RandomProcess.h"
#include <vector>

// Multi-dimensional Brownian motion driven by correlated Gaussian shocks.
class BrownianND : public RandomProcess {
public:
  // CorrelationMatrix uses lecture naming and parameterization conventions.
  BrownianND(RandomGenerator* generator, int dimension,
             std::vector<std::vector<double>>* correlationMatrix);

  // Simulates all components on [startTime, endTime] with nbSteps steps.
  void Simulate(double startTime, double endTime, size_t nbSteps) override;

protected:
  // Lecture naming is preserved: CorrelationMatrix_.
  // Numerical convention used in simulation: this matrix acts as the loading
  // matrix B in dW = B dZ. To target covariance Sigma, provide B such that
  // Sigma = B B^T. If null, identity is used (independent components).
  std::vector<std::vector<double>>* CorrelationMatrix_;
};
