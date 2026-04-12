#pragma once

#include "RandomGenerator.h"
#include "MonteCarlo/SinglePath.h"
#include <vector>

// Base interface for any simulated stochastic process.
class RandomProcess {
public:
  // Builds a process with a generator and a fixed state dimension.
  RandomProcess(RandomGenerator* generator, int dimension);
  virtual ~RandomProcess();

  // Simulates the process on [startTime, endTime] with nbSteps time steps.
  virtual void Simulate(double startTime, double endTime, size_t nbSteps) = 0;

  // Returns the simulated path for one component, or nullptr if out of range.
  SinglePath* GetPath(int dimension = 0) const;

protected:
  RandomGenerator* Generator_;
  std::vector<SinglePath*> Paths_;
  int Dimension_;
};
