#pragma once

#include "SDE/RandomProcess.h"

// Two-factor Heston stochastic-volatility model (spot, variance).
class Heston : public RandomProcess {
public:
  // Stores Heston parameters following lecture notation.
  Heston(RandomGenerator* generator, double spot, double initVariance,
         double mu, double theta, double kappa, double sigma, double rho);

  // Simulates coupled spot/variance paths on [startTime, endTime].
  void Simulate(double startTime, double endTime, size_t nbSteps) override;

private:
  double Spot_;
  double InitVariance_;
  // Lecture naming is preserved: Mu_ denotes the drift parameter in dS_t.
  // In pricing experiments under Q, Mu_ is expected to be set to r.
  double Mu_;
  double Theta_;
  double Kappa_;
  double Sigma_;
  double Rho_;
};
