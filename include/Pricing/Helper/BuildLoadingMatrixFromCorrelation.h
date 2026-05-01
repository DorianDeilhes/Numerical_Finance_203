#pragma once

#include <vector>

namespace PricingHelper {

// Builds B such that correlation_matrix = B B^T.
// Uses Cholesky when possible and falls back to symmetric diagonalization.
std::vector<std::vector<double>> BuildLoadingMatrixFromCorrelation(
    const std::vector<std::vector<double>>& correlation_matrix);

}  // namespace PricingHelper
