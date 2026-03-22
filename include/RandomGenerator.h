#pragma once

class RandomGenerator {
public:
  virtual double Generate() = 0;

  double Mean(unsigned long nbSim);

  virtual ~RandomGenerator() {}
};
