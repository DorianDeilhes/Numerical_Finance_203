#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "ContinuousGenerator.h"

// ExpoAlgo - Enum for selecting Exponential generation algorithm
enum ExpoAlgo {
  InverseDistribution,  // Inverse transform method (Slide 27)
  ExpoRejectionSampling // Rejection sampling method
};

// Exponential - Exponential distribution X ~ E(λ)
// F(x) = 1 - e^(-λx)

class Exponential : public ContinuousGenerator {
public:
  // Constructor - takes lambda, algorithm choice, and uniform generator
  Exponential(double lambda, ExpoAlgo algo, UniformGenerator *uniformGen);

  // Generate an exponential random variable
  double Generate() override;

  // Virtual destructor
  virtual ~Exponential() {}

private:
  double lambda_; // Exponential parameter
  ExpoAlgo algo_; // Algorithm selection
  UniformGenerator *uniformGen_;

  // Helper methods for each algorithm
  double GenerateInverse();
  double GenerateRejection();
};
