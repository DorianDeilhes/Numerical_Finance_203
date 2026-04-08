#include "SDE/BrownianND.h"
#include "ContinuousGenerator/Normal.h"
#include "UniformGenerator/UniformGenerator.h"
#include <cmath>
#include <stdexcept>

BrownianND::BrownianND(RandomGenerator* generator, int dimension,
                       std::vector<std::vector<double>>* correlationMatrix)
    : RandomProcess(generator, dimension), CorrelationMatrix_(correlationMatrix) {}

void BrownianND::Simulate(double startTime, double endTime, size_t nbSteps) {
  UniformGenerator* uniform = dynamic_cast<UniformGenerator*>(Generator_);
  if (uniform == nullptr) {
    throw std::runtime_error("BrownianND requires a UniformGenerator");
  }

  const double dt = (endTime - startTime) / static_cast<double>(nbSteps);
  const double sqrtDt = std::sqrt(dt);
  Normal standardNormal(0.0, 1.0, BoxMuller, uniform);

  for (int d = 0; d < Dimension_; ++d) {
    delete Paths_[static_cast<size_t>(d)];
    Paths_[static_cast<size_t>(d)] = new SinglePath(startTime, endTime, nbSteps);
    Paths_[static_cast<size_t>(d)]->InsertValue(0.0);
  }

  std::vector<double> values(Dimension_, 0.0);
  for (size_t step = 0; step < nbSteps; ++step) {
    std::vector<double> shocks(Dimension_, 0.0);
    for (int d = 0; d < Dimension_; ++d) {
      shocks[static_cast<size_t>(d)] = standardNormal.Generate();
    }

    for (int i = 0; i < Dimension_; ++i) {
      double correlatedShock = 0.0;
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
