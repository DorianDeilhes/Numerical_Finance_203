#pragma once

namespace MonteCarloHelper {

double GeometricBrownianMilsteinStep(double spot,
                                    double rate,
                                    double vol,
                                    double dt,
                                    double shock);

}  // namespace MonteCarloHelper