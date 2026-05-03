#include "DiscreteGenerator/FiniteSet.h"

#include <cmath>
#include <stdexcept>

// Constructor - computes cumulative probabilities
FiniteSet::FiniteSet(const std::vector<double> &probas,
                     UniformGenerator *uniformGen)
    : Probas(probas), uniformGen_(uniformGen) {
  if (uniformGen_ == 0) {
    throw std::invalid_argument("FiniteSet: uniform generator must not be null");
  }
  if (Probas.empty()) {
    throw std::invalid_argument("FiniteSet: probability vector must not be empty");
  }

  // Compute cumulative probabilities P_k = sum(p_1 to p_k)
  CumulativeProb.resize(Probas.size());
  double sum = 0.0;
  for (size_t i = 0; i < Probas.size(); i++) {
    if (!std::isfinite(Probas[i]) || Probas[i] < 0.0) {
      throw std::invalid_argument("FiniteSet: probabilities must be finite and non-negative");
    }
    sum += Probas[i];
    CumulativeProb[i] = sum;
  }
  if (std::fabs(sum - 1.0) > 1e-10) {
    throw std::invalid_argument("FiniteSet: probabilities must sum to 1");
  }
}

// Generate - implements finite set algorithm (Slide 22)
// Find k such that P_{k-1} <= U < P_k
double FiniteSet::Generate() {
  double U = uniformGen_->Generate();

  // Find which interval U falls into
  for (size_t k = 0; k < CumulativeProb.size(); k++) {
    if (U < CumulativeProb[k]) {
      return static_cast<double>(k + 1);
    }
  }

  // Edge case for a generator returning exactly 1.
  return static_cast<double>(CumulativeProb.size());
}
