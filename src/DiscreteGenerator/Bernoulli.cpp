#include "DiscreteGenerator/Bernoulli.h"

#include <cmath>
#include <stdexcept>

// Constructor - initializes probability and stores reference to uniform generator
Bernoulli::Bernoulli(double p, UniformGenerator *uniformGen)
    : p_(p), uniformGen_(uniformGen) {
  if (!std::isfinite(p_) || p_ < 0.0 || p_ > 1.0) {
    throw std::invalid_argument("Bernoulli: p must be in [0, 1]");
  }
  if (uniformGen_ == 0) {
    throw std::invalid_argument("Bernoulli: uniform generator must not be null");
  }
}

// Generate - implements Bernoulli algorithm from Slide 20
// Generate U ~ U[0,1], return 1 if U <= p else 0
double Bernoulli::Generate() {
  double U = uniformGen_->Generate();
  return (U <= p_) ? 1.0 : 0.0;
}
