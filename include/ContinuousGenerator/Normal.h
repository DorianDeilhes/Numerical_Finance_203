#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "ContinuousGenerator.h"

// NormalAlgo - Enum for selecting Normal generation algorithm
enum NormalAlgo {
  BoxMuller,              // Box-Muller transform (Slide 35)
  CentralLimitTheorem,    // Central Limit Theorem (Slide 36)
  NormalRejectionSampling // Rejection sampling with double exponential (Slide
                          // 40)
};

// Normal - Normal distribution X ~ N(μ, σ²)

class Normal : public ContinuousGenerator {
public:
  // Constructor - takes mean, std deviation, algorithm choice, and uniform
  // generator
  Normal(double mu, double sigma, NormalAlgo algo,
         UniformGenerator *uniformGen);

  // Generate a normal random variable
  double Generate() override;

  // Virtual destructor
  virtual ~Normal() {}

private:
  double mu_;       // Mean
  double sigma_;    // Standard deviation
  NormalAlgo algo_; // Algorithm selection
  UniformGenerator *uniformGen_;

  // Box-Muller generates two values, cache second one
  bool hasSpare_;
  double spare_;

  // Helper methods for each algorithm
  double GenerateBoxMuller();
  double GenerateCLT();
  double GenerateRejection();
};
