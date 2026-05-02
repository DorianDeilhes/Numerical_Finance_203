#pragma once

namespace MonteCarloHelper {

// Exact one-step transition for Black-Scholes under dS = (r - q) S dt + sigma S dW.
// dividend defaults to 0 for backward compatibility with non-dividend-paying code paths.
double GeometricBrownianExactStep(double spot,
                                  double rate,
                                  double vol,
                                  double dt,
                                  double shock,
                                  double dividend = 0.0);

}  // namespace MonteCarloHelper
