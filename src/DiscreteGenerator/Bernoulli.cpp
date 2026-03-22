#include "DiscreteGenerator/Bernoulli.h"

// Constructor - initializes probability and stores reference to uniform
// generator
Bernoulli::Bernoulli(double p, UniformGenerator *uniformGen)
    : p_(p), uniformGen_(uniformGen) {}

// Generate - implements Bernoulli algorithm from Slide 20
// Generate U ~ U[0,1], return 1 if U <= p else 0
double Bernoulli::Generate() {
  double U = uniformGen_->Generate(); // Get uniform random number
  return (U <= p_) ? 1.0 : 0.0;       // Return 1 (success) or 0 (failure)
}
