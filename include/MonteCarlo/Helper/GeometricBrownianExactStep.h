#pragma once

namespace MonteCarloHelper {

// Exact one-step transition for Black-Scholes under dS = r S dt + sigma S dW.
double GeometricBrownianExactStep(double spot,
                                  double rate,
                                  double vol,
                                  double dt,
                                  double shock);

}  // namespace MonteCarloHelper
