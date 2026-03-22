#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "DiscreteGenerator.h"
#include <vector>


// FiniteSet - Random variable on finite set {1, 2, ..., K} with given
// probabilities Algorithm (Slide 22): Divide [0,1] into K intervals, find which
// interval U falls into

class FiniteSet : public DiscreteGenerator {
public:
  // Constructor - takes vector of probabilities and uniform generator
  // probas[i] = P(X = i+1), must sum to 1.0
  FiniteSet(const std::vector<double> &probas, UniformGenerator *uniformGen);

  // Generate a value from the finite set
  double Generate() override;

  // Virtual destructor
  virtual ~FiniteSet() {}

private:
  std::vector<double> Probas;         // Probabilities p_k
  std::vector<double> CumulativeProb; // Cumulative probabilities P_k
  UniformGenerator *uniformGen_;
};
