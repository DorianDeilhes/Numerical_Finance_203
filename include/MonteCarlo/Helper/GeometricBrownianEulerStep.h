#pragma once

namespace MonteCarloHelper {

// dividend defaults to 0 for backward compatibility with non-dividend-paying code paths.
double GeometricBrownianEulerStep(double spot,
                                  double rate,
                                  double vol,
                                  double dt,
                                  double shock,
                                  double dividend = 0.0);

}  // namespace MonteCarloHelper