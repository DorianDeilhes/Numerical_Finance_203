#include "DiscreteGenerator/Binomial.h"

// Constructor
Binomial::Binomial(int n, double p, UniformGenerator *uniformGen)
    : n_(n), p_(p), uniformGen_(uniformGen) {}

// Generate - implements Binomial as sum of n Bernoulli (Slide 21)
// X = Sum of n independent Bernoulli(p) variables
double Binomial::Generate() {
  int successes = 0;
  for (int i = 0; i < n_; i++) {
    double U = uniformGen_->Generate();
    if (U <= p_) {
      successes++; // Success
    }
  }
  return static_cast<double>(successes);
}
