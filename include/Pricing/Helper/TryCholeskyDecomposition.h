#pragma once

#include <vector>

namespace PricingHelper {

// Attempts a Cholesky factorization A = L L^T.
// Returns false when A is symmetric but not strictly positive definite.
bool TryCholeskyDecomposition(const std::vector<std::vector<double>>& matrix,
                              std::vector<std::vector<double>>* lower);

}  // namespace PricingHelper
