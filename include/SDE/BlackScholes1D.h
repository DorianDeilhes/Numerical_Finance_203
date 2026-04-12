#pragma once

#include "SDE/Brownian1D.h"

// Base parameter container for one-asset Black-Scholes dynamics.
class BlackScholes1D : public RandomProcess {
public:
  // Stores model parameters used by 1D Black-Scholes simulation schemes.
  BlackScholes1D(RandomGenerator* generator, double spot, double rate,
                 double vol);

protected:
  double Spot_;
  double Rate_;
  double Vol_;
};
