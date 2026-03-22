#include "ContinuousGenerator/BivariateNormal.h"
#include <cmath>

// Constructor
BivariateNormal::BivariateNormal(double mu_x, double mu_y, double sigma_x,
                                 double sigma_y, double rho, NormalAlgo algo,
                                 UniformGenerator *uniformGen)
    : mu_x_(mu_x), mu_y_(mu_y), sigma_x_(sigma_x), sigma_y_(sigma_y), rho_(rho),
      standardNormal_(0.0, 1.0, algo, uniformGen) {}

// Generate one bivariate Gaussian sample.
std::pair<double, double> BivariateNormal::Generate() {
  // Step 1: generate Z1, Z2 ~ N(0,1), independent.
  double Z1 = standardNormal_.Generate();
  double Z2 = standardNormal_.Generate();

  // Step 2: apply the lecture transformation.
  double X = sigma_x_ * Z1 + mu_x_;

  // Corr(X, Y) = rho by construction.
  double Y = sigma_y_ * (rho_ * Z1 + sqrt(1.0 - rho_ * rho_) * Z2) + mu_y_;

  return std::make_pair(X, Y);
}

// Empirical mean of X.
double BivariateNormal::MeanFirst(unsigned long nbSim) {
  double sum = 0.0;

  for (unsigned long i = 0; i < nbSim; i++) {
    sum += Generate().first;
  }

  return sum / nbSim;
}

// Empirical mean of Y.
double BivariateNormal::MeanSecond(unsigned long nbSim) {
  double sum = 0.0;

  for (unsigned long i = 0; i < nbSim; i++) {
    sum += Generate().second;
  }

  return sum / nbSim;
}

// Empirical correlation Corr(X, Y).
double BivariateNormal::Correlation(unsigned long nbSim) {
  double sumX = 0.0;
  double sumY = 0.0;
  double sumX2 = 0.0;
  double sumY2 = 0.0;
  double sumXY = 0.0;

  for (unsigned long i = 0; i < nbSim; i++) {
    std::pair<double, double> sample = Generate();
    double x = sample.first;
    double y = sample.second;

    sumX += x;
    sumY += y;
    sumX2 += x * x;
    sumY2 += y * y;
    sumXY += x * y;
  }

  double meanX = sumX / nbSim;
  double meanY = sumY / nbSim;
  double varX = sumX2 / nbSim - meanX * meanX;
  double varY = sumY2 / nbSim - meanY * meanY;
  double covXY = sumXY / nbSim - meanX * meanY;

  if (varX <= 0.0 || varY <= 0.0) {
    return 0.0;
  }

  return covXY / sqrt(varX * varY);
}