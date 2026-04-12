#pragma once

#include "SDE/BrownianND.h"

// Base parameter container for two-asset correlated Black-Scholes dynamics.
class BlackScholes2D : public RandomProcess {
public:
  // Stores model parameters used by 2D Black-Scholes simulation schemes.
  BlackScholes2D(RandomGenerator* generator, double spot1, double spot2,
                 double rate1, double rate2, double vol1, double vol2,
                 double rho);

protected:
  double Spot1_;
  double Spot2_;
  double Rate1_;
  double Rate2_;
  double Vol1_;
  double Vol2_;
  double Rho_;
};
