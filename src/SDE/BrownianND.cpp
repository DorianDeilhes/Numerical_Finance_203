#include "SDE/BrownianND.h"
#include "MonteCarlo/Helper/GetUniformGeneratorOrThrow.h"
#include "MonteCarlo/Helper/ValidateTimeGrid.h"
#include "SDE/Helper/ResetAllPaths.h"
#include "SDE/Helper/ValidateSquareLoadingMatrix.h"
#include "ContinuousGenerator/Normal.h"
#include <cmath>
#include <stdexcept>

BrownianND::BrownianND(RandomGenerator* generator, int dimension,
                       std::vector<std::vector<double>>* correlationMatrix)
    : RandomProcess(generator, dimension), CorrelationMatrix_(correlationMatrix) {}

void BrownianND::Simulate(double startTime, double endTime, size_t nbSteps) {
  MonteCarloHelper::ValidateTimeGrid("BrownianND::Simulate", startTime, endTime, nbSteps);
  SDEHelper::ValidateSquareLoadingMatrix(CorrelationMatrix_, static_cast<size_t>(Dimension_),
                                         "BrownianND::Simulate");

  UniformGenerator* uniform =
      MonteCarloHelper::GetUniformGeneratorOrThrow(Generator_, "BrownianND::Simulate");

  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);
  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);

  // Reset one path per Brownian component.
  SDEHelper::ResetAllPaths(Paths_, static_cast<size_t>(Dimension_), startTime, endTime,
                           nbSteps, 0.0);

  std::vector<double> values(Dimension_, 0.0);
  for (size_t step = 0; step < nbSteps; ++step) {
    std::vector<double> shocks(Dimension_, 0.0);
    for (int d = 0; d < Dimension_; ++d) {
      shocks[static_cast<size_t>(d)] = standardNormal.Generate();
    }

    // Component i uses row i of CorrelationMatrix_ to build correlated shock.
    for (int i = 0; i < Dimension_; ++i) {
      double correlatedShock = 0.0;
      // CorrelationMatrix_ is used as the Gaussian loading matrix in practice.
      for (int j = 0; j < Dimension_; ++j) {
        const double corr = (CorrelationMatrix_ != nullptr)
                                ? (*CorrelationMatrix_)[static_cast<size_t>(i)][static_cast<size_t>(j)]
                                : (i == j ? 1.0 : 0.0);
        correlatedShock += corr * shocks[static_cast<size_t>(j)];
      }
      values[static_cast<size_t>(i)] += sqrtDt * correlatedShock;
      Paths_[static_cast<size_t>(i)]->InsertValue(values[static_cast<size_t>(i)]);
    }
  }
}
