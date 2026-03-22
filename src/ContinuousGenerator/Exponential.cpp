#include "ContinuousGenerator/Exponential.h"
#include <cmath>

// Constructor
Exponential::Exponential(double lambda, ExpoAlgo algo,
                         UniformGenerator *uniformGen)
    : lambda_(lambda), algo_(algo), uniformGen_(uniformGen) {}

// Generate - dispatches to selected algorithm
double Exponential::Generate() {
  if (algo_ == InverseDistribution) {
    return GenerateInverse();
  } else {
    return GenerateRejection();
  }
}

// Inverse Distribution Method (Slide 27)
// F(x) = 1 - e^(-λx) => F^(-1)(u) = -ln(1-u)/λ
// Since U ~ U[0,1] => (1-U) ~ U[0,1], we can use: X = -ln(U)/λ
double Exponential::GenerateInverse() {
  double U = uniformGen_->Generate();
  return -log(U) / lambda_;
}

// Rejection Sampling Method
// This is a simple implementation using the compact support [0, M]
// where M is chosen large enough
double Exponential::GenerateRejection() {
  // For exponential, inverse method is more efficient
  // This is included for completeness
  // We'll use a simple rejection with envelope M*uniform
  double M = lambda_; // Maximum of λe^(-λx) at x=0

  while (true) {
    double U1 = uniformGen_->Generate();
    double U2 = uniformGen_->Generate();

    double X = -log(U1) / lambda_; // Proposal using inverse
    double density = lambda_ * exp(-lambda_ * X);

    if (U2 * M <= density) {
      return X;
    }
  }
}
