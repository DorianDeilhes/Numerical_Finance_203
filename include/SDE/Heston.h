#pragma once

#include "SDE/RandomProcess.h"

class Heston : public RandomProcess {
public:
  Heston(RandomGenerator* generator, double spot, double initVariance,
         double mu, double theta, double kappa, double sigma, double rho);
  void Simulate(double startTime, double endTime, size_t nbSteps) override;

private:
  double Spot_;
  double InitVariance_;
  double Mu_;
  double Theta_;
  double Kappa_;
  double Sigma_;
  double Rho_;
};
