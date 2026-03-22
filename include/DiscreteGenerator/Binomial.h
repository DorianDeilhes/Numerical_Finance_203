#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "DiscreteGenerator.h"


// Binomial - Sum of n independent Bernoulli(p) trials
// X ~ B(n,p) represents number of successes in n trials
// Algorithm (Slide 21): Sum n Bernoulli variables

class Binomial : public DiscreteGenerator {
public:
  // Constructor - takes n (trials), p (probability), and uniform generator
  Binomial(int n, double p, UniformGenerator *uniformGen);

  // Generate a binomial random variable
  double Generate() override;

  // Virtual destructor
  virtual ~Binomial() {}

private:
  int n_;                        // Number of trials
  double p_;                     // Probability of success
  UniformGenerator *uniformGen_; // Composition: uses a uniform generator
};
