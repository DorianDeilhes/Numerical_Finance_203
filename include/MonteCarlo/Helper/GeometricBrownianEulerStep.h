#pragma once

namespace MonteCarloHelper {

double GeometricBrownianEulerStep(double spot,
                                  double rate,
                                  double vol,
                                  double dt,
                                  double shock);

}  // namespace MonteCarloHelper