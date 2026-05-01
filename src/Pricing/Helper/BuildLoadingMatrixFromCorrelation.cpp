#include "Pricing/Helper/BuildLoadingMatrixFromCorrelation.h"

#include "Pricing/Helper/JacobiEigenDecomposition.h"
#include "Pricing/Helper/TryCholeskyDecomposition.h"
#include "Pricing/Helper/ValidateCorrelationMatrix.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

std::vector<std::vector<double>> BuildLoadingMatrixFromCorrelation(
    const std::vector<std::vector<double>>& correlation_matrix) {
  const size_t dimension = correlation_matrix.size();
  if (dimension == 0) {
    throw std::runtime_error("BuildLoadingMatrixFromCorrelation requires dimension > 0");
  }
  ValidateCorrelationMatrix(correlation_matrix, dimension);

  std::vector<std::vector<double>> loading_matrix;
  if (TryCholeskyDecomposition(correlation_matrix, &loading_matrix)) {
    return loading_matrix;
  }

  std::vector<double> eigenvalues;
  std::vector<std::vector<double>> eigenvectors;
  if (!JacobiEigenDecomposition(correlation_matrix, &eigenvalues, &eigenvectors)) {
    throw std::runtime_error("BuildLoadingMatrixFromCorrelation failed to diagonalize matrix");
  }

  const double tolerance = 1e-10;
  loading_matrix.assign(dimension, std::vector<double>(dimension, 0.0));
  for (size_t j = 0; j < dimension; ++j) {
    if (eigenvalues[j] < -tolerance) {
      throw std::runtime_error("BuildLoadingMatrixFromCorrelation requires a positive semidefinite matrix");
    }

    const double clipped_eigenvalue = eigenvalues[j] > 0.0 ? eigenvalues[j] : 0.0;
    const double sqrt_eigenvalue = std::sqrt(clipped_eigenvalue);
    for (size_t i = 0; i < dimension; ++i) {
      loading_matrix[i][j] = eigenvectors[i][j] * sqrt_eigenvalue;
    }
  }

  return loading_matrix;
}

}  // namespace PricingHelper
