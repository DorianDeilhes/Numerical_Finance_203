#include "DiscreteGenerator/FiniteSet.h"

// Constructor - computes cumulative probabilities
FiniteSet::FiniteSet(const std::vector<double> &probas,
                     UniformGenerator *uniformGen)
    : Probas(probas), uniformGen_(uniformGen) {

  // Compute cumulative probabilities P_k = sum(p_1 to p_k)
  CumulativeProb.resize(Probas.size());
  double sum = 0.0;
  for (size_t i = 0; i < Probas.size(); i++) {
    sum += Probas[i];
    CumulativeProb[i] = sum;
  }
}

// Generate - implements finite set algorithm (Slide 22)
// Find k such that P_{k-1} <= U < P_k
double FiniteSet::Generate() {
  double U = uniformGen_->Generate();

  // Find which interval U falls into
  for (size_t k = 0; k < CumulativeProb.size(); k++) {
    if (U < CumulativeProb[k]) {
      return static_cast<double>(k + 1); // Return value k+1 (1-indexed)
    }
  }

  // Should not reach here if probabilities sum to 1, but handle edge case
  return static_cast<double>(CumulativeProb.size());
}
