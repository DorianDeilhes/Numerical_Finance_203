#include "MonteCarlo/Helper/GeometricBrownianMilsteinStep.h"

#include <cmath>

namespace MonteCarloHelper {

double GeometricBrownianMilsteinStep(double spot,
                                    double rate,
                                    double vol,
                                    double dt,
                                    double shock) {
  return spot + rate * spot * dt + vol * spot * std::sqrt(dt) * shock +
         0.5 * vol * vol * spot * (shock * shock - 1.0) * dt;
}

}  // namespace MonteCarloHelper