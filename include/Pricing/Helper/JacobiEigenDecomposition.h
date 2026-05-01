#pragma once

#include <vector>

namespace PricingHelper {

// Diagonalizes a real symmetric matrix: A = O D O^T.
// Eigenvectors are returned as columns of eigenvectors.
bool JacobiEigenDecomposition(const std::vector<std::vector<double>>& matrix,
                              std::vector<double>* eigenvalues,
                              std::vector<std::vector<double>>* eigenvectors);

}  // namespace PricingHelper
