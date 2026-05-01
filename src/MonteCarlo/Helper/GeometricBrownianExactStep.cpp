#include "MonteCarlo/Helper/GeometricBrownianExactStep.h"

#include <cmath>

namespace MonteCarloHelper {

double GeometricBrownianExactStep(double spot,
                                  double rate,
                                  double vol,
                                  double dt,
                                  double shock) {
  const double drift = (rate - 0.5 * vol * vol) * dt;
  const double diffusion = vol * std::sqrt(dt) * shock;
  return spot * std::exp(drift + diffusion);
}

}  // namespace MonteCarloHelper
