#include "Pricing/Helper/SolveLinearSystem3x3.h"

#include <cmath>

namespace PricingHelper {

bool SolveLinearSystem3x3(const std::array<std::array<double, 3>, 3>& matrix,
                          const std::array<double, 3>& rhs,
                          std::array<double, 3>* solution) {
  if (solution == nullptr) {
    return false;
  }

  std::array<std::array<double, 4>, 3> augmented = {{
      {{matrix[0][0], matrix[0][1], matrix[0][2], rhs[0]}},
      {{matrix[1][0], matrix[1][1], matrix[1][2], rhs[1]}},
      {{matrix[2][0], matrix[2][1], matrix[2][2], rhs[2]}},
  }};

  // Gaussian elimination with partial pivoting for numerical stability.
  for (size_t col = 0; col < 3; ++col) {
    size_t pivot_row = col;
    double pivot_abs = std::fabs(augmented[pivot_row][col]);
    for (size_t row = col + 1; row < 3; ++row) {
      const double candidate_abs = std::fabs(augmented[row][col]);
      if (candidate_abs > pivot_abs) {
        pivot_abs = candidate_abs;
        pivot_row = row;
      }
    }

    if (pivot_abs < 1e-12) {
      return false;
    }

    if (pivot_row != col) {
      const std::array<double, 4> temp = augmented[col];
      augmented[col] = augmented[pivot_row];
      augmented[pivot_row] = temp;
    }

    const double pivot = augmented[col][col];
    for (size_t k = col; k < 4; ++k) {
      augmented[col][k] /= pivot;
    }

    for (size_t row = 0; row < 3; ++row) {
      if (row == col) {
        continue;
      }
      const double factor = augmented[row][col];
      for (size_t k = col; k < 4; ++k) {
        augmented[row][k] -= factor * augmented[col][k];
      }
    }
  }

  (*solution)[0] = augmented[0][3];
  (*solution)[1] = augmented[1][3];
  (*solution)[2] = augmented[2][3];
  return true;
}

}  // namespace PricingHelper
