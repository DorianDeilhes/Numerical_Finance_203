#pragma once

#include "RandomGenerator.h"
#include "MonteCarlo/SinglePath.h"
#include <vector>

class RandomProcess {
public:
  RandomProcess(RandomGenerator* generator, int dimension);
  virtual ~RandomProcess();

  virtual void Simulate(double startTime, double endTime, size_t nbSteps) = 0;

  SinglePath* GetPath(int dimension = 0) const;

protected:
  RandomGenerator* Generator_;
  std::vector<SinglePath*> Paths_;
  int Dimension_;
};
