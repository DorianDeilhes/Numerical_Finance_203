#include "DiscreteGenerator/Poisson.h"
#include <cmath>

// Constructor
Poisson::Poisson(double lambda, PoissonAlgo algo, UniformGenerator *uniformGen)
    : lambda_(lambda), algo_(algo), uniformGen_(uniformGen) {}

// Generate - dispatches to selected algorithm
double Poisson::Generate() {
  if (algo_ == FirstAlgorithm) {
    return GenerateFirstAlgorithm();
  } else {
    return GenerateSecondAlgorithm();
  }
}

// First Algorithm (Slide 23) - Uses cumulative probabilities
// p_k = e^(-λ) * λ^k / k!
// p_{k+1} = (λ/(k+1)) * p_k
// P_k = sum of p_0 to p_k
double Poisson::GenerateFirstAlgorithm() {
  double U = uniformGen_->Generate();

  double p = exp(-lambda_); // p_0 = e^(-λ)
  double P = p;             // P_0 = p_0
  int k = 0;

  // Find k such that P_{k-1} <= U < P_k
  while (U >= P) {
    k++;
    p = (lambda_ / k) * p; // p_k = (λ/k) * p_{k-1}
    P = P + p;             // P_k = P_{k-1} + p_k
  }

  return static_cast<double>(k);
}

// Second Algorithm (Slide 24) - Uses exponential relationship
// If Y_i ~ E(λ), then number of events in unit time ~ P(λ)
// Generate exponentials until sum exceeds 1
double Poisson::GenerateSecondAlgorithm() {
  double sum = 0.0;
  int k = 0;

  // Sum exponentials: Z_k = Y_1 + Y_2 + ... + Y_k
  // Find k such that Z_k <= 1 < Z_{k+1}
  while (sum <= 1.0) {
    // Generate Y ~ E(λ) using inverse transform: Y = -ln(U)/λ
    double U = uniformGen_->Generate();
    double Y = -log(U) / lambda_;
    sum += Y;
    k++;
  }

  return static_cast<double>(k - 1); // Return k-1 since we exceeded 1
}
