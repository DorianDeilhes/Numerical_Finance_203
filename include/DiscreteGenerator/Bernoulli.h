#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "DiscreteGenerator.h"


// Bernoulli - Generates binary outcomes (0 or 1) with probability p
// X ~ B(p) where P(X = 1) = p and P(X = 0) = 1 - p
// Algorithm: Generate U ~ U[0,1], return 1 if U <= p, else 0

class Bernoulli : public DiscreteGenerator {
public:
  // Constructor - takes probability p and a uniform generator
  Bernoulli(double p, UniformGenerator *uniformGen);

  // Generate a Bernoulli random variable (0 or 1)
  double Generate() override;

  // Virtual destructor
  virtual ~Bernoulli() {}

private:
  double p_;                     // Probability of success
  UniformGenerator *uniformGen_; // Composition: uses a uniform generator
};
