#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "DiscreteGenerator.h"


// PoissonAlgo - Enum for selecting Poisson generation algorithm
enum PoissonAlgo {
  FirstAlgorithm, // Cumulative probabilities method (Slide 23)
  SecondAlgorithm // Exponential relationship method (Slide 24)
};

// Poisson - Poisson distribution X ~ P(λ)
// P(X = k) = e^(-λ) * λ^k / k!

class Poisson : public DiscreteGenerator {
public:
  // Constructor - takes lambda, algorithm choice, and uniform generator
  Poisson(double lambda, PoissonAlgo algo, UniformGenerator *uniformGen);

  // Generate a Poisson random variable
  double Generate() override;

  // Virtual destructor
  virtual ~Poisson() {}

private:
  double lambda_;    // Poisson parameter
  PoissonAlgo algo_; // Algorithm selection
  UniformGenerator *uniformGen_;

  // Helper methods for each algorithm
  double GenerateFirstAlgorithm();
  double GenerateSecondAlgorithm();
};
