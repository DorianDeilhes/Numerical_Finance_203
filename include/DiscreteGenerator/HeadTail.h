#pragma once

#include "../UniformGenerator/UniformGenerator.h"
#include "DiscreteGenerator.h"


// HeadTail - Simulates a coin flip (Head or Tail with equal probability)
// Special case of Bernoulli with p = 0.5
// Algorithm (Slide 19): Generate U ~ U[0,1], return 1 if U <= 0.5 else 0

class HeadTail : public DiscreteGenerator {
public:
  // Constructor - takes a uniform generator
  HeadTail(UniformGenerator *uniformGen);

  // Generate a coin flip (0 = Tail, 1 = Head)
  double Generate() override;

  // Virtual destructor
  virtual ~HeadTail() {}

private:
  UniformGenerator *uniformGen_; // Composition: uses a uniform generator
};
