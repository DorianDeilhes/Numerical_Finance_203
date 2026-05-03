#include "DiscreteGenerator/Binomial.h"

#include <cmath>
#include <stdexcept>

// Constructor
Binomial::Binomial(int n, double p, UniformGenerator *uniformGen)
    : n_(n), p_(p), uniformGen_(uniformGen) {
  if (n_ < 0) {
    throw std::invalid_argument("Binomial: n must be non-negative");
  }
  if (!std::isfinite(p_) || p_ < 0.0 || p_ > 1.0) {
    throw std::invalid_argument("Binomial: p must be in [0, 1]");
  }
  if (uniformGen_ == 0) {
    throw std::invalid_argument("Binomial: uniform generator must not be null");
  }
}

// Generate - implements Binomial as sum of n Bernoulli (Slide 21)
// X = Sum of n independent Bernoulli(p) variables
double Binomial::Generate() {
  int successes = 0;
  for (int i = 0; i < n_; i++) {
    double U = uniformGen_->Generate();
    if (U <= p_) {
      successes++;
    }
  }
  return static_cast<double>(successes);
}
