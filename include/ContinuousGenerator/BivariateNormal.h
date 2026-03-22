#pragma once

#include "Normal.h"
#include <utility>

// BivariateNormal - Bivariate Gaussian vector (X, Y)
//
// We generate two independent standard normals Z1 and Z2, then apply:
// X = mu_x + sigma_x * Z1
// Y = mu_y + sigma_y * (rho * Z1 + sqrt(1-rho^2) * Z2)
//
// This gives Corr(X, Y) = rho.

class BivariateNormal {
public:
  // Constructor - takes both means, both standard deviations,
  // the correlation rho, the Normal algorithm to use, and a uniform generator.
  BivariateNormal(double mu_x, double mu_y, double sigma_x, double sigma_y,
                  double rho, NormalAlgo algo, UniformGenerator *uniformGen);

  // Generate one pair (X, Y).
  std::pair<double, double> Generate();

  // Empirical mean of the first component.
  double MeanFirst(unsigned long nbSim);

  // Empirical mean of the second component.
  double MeanSecond(unsigned long nbSim);

  // Empirical correlation between the two components.
  double Correlation(unsigned long nbSim);

private:
  double mu_x_;
  double mu_y_;
  double sigma_x_;
  double sigma_y_;
  double rho_;

  // We reuse the existing Normal generator to keep the implementation simple.
  // When Box-Muller is used, the second call can reuse the cached spare value.
  Normal standardNormal_;
};