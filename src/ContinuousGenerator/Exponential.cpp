#include "ContinuousGenerator/Exponential.h"

#include <cmath>
#include <stdexcept>

// Constructor
Exponential::Exponential(double lambda, ExpoAlgo algo,
                         UniformGenerator *uniformGen)
    : lambda_(lambda), algo_(algo), uniformGen_(uniformGen) {
  if (!std::isfinite(lambda_) || lambda_ <= 0.0) {
    throw std::invalid_argument("Exponential: lambda must be finite and positive");
  }
  if (algo_ != InverseDistribution && algo_ != ExpoRejectionSampling) {
    throw std::invalid_argument("Exponential: unknown generation algorithm");
  }
  if (uniformGen_ == 0) {
    throw std::invalid_argument("Exponential: uniform generator must not be null");
  }
}

// Generate - dispatches to selected algorithm
double Exponential::Generate() {
  if (algo_ == InverseDistribution) {
    return GenerateInverse();
  } else {
    return GenerateRejection();
  }
}

// Inverse Distribution Method (Slide 27)
// F(x) = 1 - exp(-lambda x), so F^(-1)(u) = -ln(1-u)/lambda.
// Since 1-U has the same law as U, we use X = -ln(U)/lambda.
double Exponential::GenerateInverse() {
  double U = uniformGen_->Generate();
  while (U <= 0.0) {
    U = uniformGen_->Generate();
  }
  return -log(U) / lambda_;
}

// Rejection Sampling Method
// Proposal: exponential distribution with parameter beta = lambda / 2.
// Since f(x) <= M g(x), with M = lambda / beta = 2, this gives an exact
// rejection sampler for the exponential distribution.
double Exponential::GenerateRejection() {
  const double proposal_lambda = 0.5 * lambda_;
  const double envelope_constant = lambda_ / proposal_lambda;

  while (true) {
    double U1 = uniformGen_->Generate();
    while (U1 <= 0.0) {
      U1 = uniformGen_->Generate();
    }
    double U2 = uniformGen_->Generate();

    double X = -log(U1) / proposal_lambda;
    double target_density = lambda_ * exp(-lambda_ * X);
    double proposal_density = proposal_lambda * exp(-proposal_lambda * X);

    if (U2 * envelope_constant * proposal_density <= target_density) {
      return X;
    }
  }
}
