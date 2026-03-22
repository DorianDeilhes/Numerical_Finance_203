#include "DiscreteGenerator/HeadTail.h"

// Constructor
HeadTail::HeadTail(UniformGenerator *uniformGen) : uniformGen_(uniformGen) {}

// Generate - implements Head or Tail algorithm (Slide 19)
// U <= 0.5 => Head (1), U > 0.5 => Tail (0)
double HeadTail::Generate() {
  double U = uniformGen_->Generate();
  return (U <= 0.5) ? 1.0 : 0.0;
}
