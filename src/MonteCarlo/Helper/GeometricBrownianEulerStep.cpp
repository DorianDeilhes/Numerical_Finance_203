#include "MonteCarlo/Helper/GeometricBrownianEulerStep.h"

#include <cmath>

namespace MonteCarloHelper {

double GeometricBrownianEulerStep(double spot,
                                  double rate,
                                  double vol,
                                  double dt,
                                  double shock,
                                  double dividend) {
  return spot + (rate - dividend) * spot * dt + vol * spot * std::sqrt(dt) * shock;
}

}  // namespace MonteCarloHelper