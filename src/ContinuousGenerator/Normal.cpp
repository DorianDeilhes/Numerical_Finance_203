#include "ContinuousGenerator/Normal.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
Normal::Normal(double mu, double sigma, NormalAlgo algo,
               UniformGenerator *uniformGen)
    : mu_(mu), sigma_(sigma), algo_(algo), uniformGen_(uniformGen),
      hasSpare_(false), spare_(0.0) {}

// Generate - dispatches to selected algorithm
double Normal::Generate() {
  if (algo_ == BoxMuller) {
    return GenerateBoxMuller();
  } else if (algo_ == CentralLimitTheorem) {
    return GenerateCLT();
  } else {
    return GenerateRejection();
  }
}

// Box-Muller Method (Slide 35)
// Generate two independent N(0,1) from two uniform:
// R = sqrt(-2*ln(U1)), Θ = 2π*U2
// X = R*cos(Θ), Y = R*sin(Θ)
// Then Z = μ + σ*X ~ N(μ, σ²)
double Normal::GenerateBoxMuller() {
  // Use spare value if available
  if (hasSpare_) {
    hasSpare_ = false;
    return mu_ + sigma_ * spare_;
  }

  // Generate two uniform values
  double U1 = uniformGen_->Generate();
  double U2 = uniformGen_->Generate();

  // Box-Muller transform
  double R = sqrt(-2.0 * log(U1));
  double Theta = 2.0 * M_PI * U2;

  // Generate two independent N(0,1)
  double X = R * cos(Theta);
  double Y = R * sin(Theta);

  // Save spare for next call
  spare_ = Y;
  hasSpare_ = true;

  // Return N(μ, σ²)
  return mu_ + sigma_ * X;
}

// Central Limit Theorem Method (Slide 36)
// Sum of 12 U[0,1] has mean 6 and variance 1
// X = (sum of 12 uniforms) - 6 ~ N(0,1)
// Then Z = μ + σ*X ~ N(μ, σ²)
double Normal::GenerateCLT() {
  double sum = 0.0;

  // Sum 12 independent U[0,1] variables
  for (int i = 0; i < 12; i++) {
    sum += uniformGen_->Generate();
  }

  // X ~ N(0,1) approximately
  double X = sum - 6.0;

  // Return N(μ, σ²)
  return mu_ + sigma_ * X;
}

// Rejection Sampling with Double Exponential (Slide 40)
// Normal density φ(x) <= sqrt(2e/π) * g(x)
// where g(x) = (1/2)e^(-|x|) is double exponential density
double Normal::GenerateRejection() {
  double a = sqrt(2.0 * exp(1.0) / M_PI); // Constant from slide

  while (true) {
    // Generate from double exponential: sign * E(1)
    double U1 = uniformGen_->Generate();
    double sign = (U1 < 0.5) ? -1.0 : 1.0;

    // Generate E(1) using inverse transform
    double U2 = uniformGen_->Generate();
    double X = -log(U2) * sign; // Double exponential

    // Acceptance/rejection
    double U3 = uniformGen_->Generate();
    double phi = exp(-X * X / 2.0) / sqrt(2.0 * M_PI); // Normal density
    double g = 0.5 * exp(-fabs(X));                    // Double exp density

    if (U3 * a * g <= phi) {
      // Accepted: transform to N(μ, σ²)
      return mu_ + sigma_ * X;
    }
  }
}
