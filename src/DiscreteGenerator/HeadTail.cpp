#include "DiscreteGenerator/HeadTail.h"

#include <stdexcept>

// Constructor
HeadTail::HeadTail(UniformGenerator *uniformGen) : uniformGen_(uniformGen) {
  if (uniformGen_ == 0) {
    throw std::invalid_argument("HeadTail: uniform generator must not be null");
  }
}

// Generate - implements Head or Tail algorithm (Slide 19)
// U <= 0.5 => Head (1), U > 0.5 => Tail (0)
double HeadTail::Generate() {
  double U = uniformGen_->Generate();
  return (U <= 0.5) ? 1.0 : 0.0;
}
