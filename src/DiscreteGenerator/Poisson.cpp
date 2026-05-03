#include "DiscreteGenerator/Poisson.h"

#include <cmath>
#include <stdexcept>

// Constructor
Poisson::Poisson(double lambda, PoissonAlgo algo, UniformGenerator *uniformGen)
    : lambda_(lambda), algo_(algo), uniformGen_(uniformGen) {
  if (!std::isfinite(lambda_) || lambda_ <= 0.0) {
    throw std::invalid_argument("Poisson: lambda must be finite and positive");
  }
  if (algo_ != FirstAlgorithm && algo_ != SecondAlgorithm) {
    throw std::invalid_argument("Poisson: unknown generation algorithm");
  }
  if (uniformGen_ == 0) {
    throw std::invalid_argument("Poisson: uniform generator must not be null");
  }
}

// Generate - dispatches to selected algorithm
double Poisson::Generate() {
  if (algo_ == FirstAlgorithm) {
    return GenerateFirstAlgorithm();
  } else {
    return GenerateSecondAlgorithm();
  }
}

// First Algorithm (Slide 23) - Uses cumulative probabilities
// p_k = exp(-lambda) * lambda^k / k!
// p_{k+1} = (lambda/(k+1)) * p_k
// P_k = sum of p_0 to p_k
double Poisson::GenerateFirstAlgorithm() {
  double U = uniformGen_->Generate();

  double p = exp(-lambda_);
  if (p == 0.0) {
    throw std::runtime_error("Poisson: lambda is too large for cumulative algorithm");
  }
  double P = p;
  int k = 0;

  // Find k such that P_{k-1} <= U < P_k
  while (U >= P) {
    k++;
    p = (lambda_ / k) * p;
    if (p == 0.0) {
      throw std::runtime_error("Poisson: cumulative algorithm underflow");
    }
    P = P + p;
  }

  return static_cast<double>(k);
}

// Second Algorithm (Slide 24) - Uses exponential relationship
// If Y_i ~ E(lambda), then the number of events in unit time follows P(lambda).
// Generate exponentials until their sum exceeds 1.
double Poisson::GenerateSecondAlgorithm() {
  double sum = 0.0;
  int k = 0;

  // Sum exponentials: Z_k = Y_1 + Y_2 + ... + Y_k
  // Find k such that Z_k <= 1 < Z_{k+1}
  while (sum <= 1.0) {
    double U = uniformGen_->Generate();
    while (U <= 0.0) {
      U = uniformGen_->Generate();
    }
    double Y = -log(U) / lambda_;
    sum += Y;
    k++;
  }

  return static_cast<double>(k - 1);
}
