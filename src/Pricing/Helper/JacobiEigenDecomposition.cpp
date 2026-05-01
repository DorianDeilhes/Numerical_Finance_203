#include "Pricing/Helper/JacobiEigenDecomposition.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace {

void ValidateSquareFiniteMatrix(const std::vector<std::vector<double>>& matrix,
                                const char* function_name) {
  const size_t dimension = matrix.size();
  if (dimension == 0) {
    throw std::runtime_error(std::string(function_name) + " requires dimension > 0");
  }

  for (size_t i = 0; i < dimension; ++i) {
    if (matrix[i].size() != dimension) {
      throw std::runtime_error(std::string(function_name) + " requires a square matrix");
    }
    for (size_t j = 0; j < dimension; ++j) {
      if (!std::isfinite(matrix[i][j])) {
        throw std::runtime_error(std::string(function_name) + " requires finite entries");
      }
    }
  }
}

std::vector<std::vector<double>> IdentityMatrix(size_t dimension) {
  std::vector<std::vector<double>> identity(dimension, std::vector<double>(dimension, 0.0));
  for (size_t i = 0; i < dimension; ++i) {
    identity[i][i] = 1.0;
  }
  return identity;
}

}  // namespace

namespace PricingHelper {

bool JacobiEigenDecomposition(const std::vector<std::vector<double>>& matrix,
                              std::vector<double>* eigenvalues,
                              std::vector<std::vector<double>>* eigenvectors) {
  if (eigenvalues == nullptr || eigenvectors == nullptr) {
    throw std::runtime_error("JacobiEigenDecomposition requires non-null outputs");
  }
  ValidateSquareFiniteMatrix(matrix, "JacobiEigenDecomposition");

  const size_t dimension = matrix.size();
  const double tolerance = 1e-12;
  const size_t max_iterations = 100 * dimension * dimension;

  std::vector<std::vector<double>> a = matrix;
  std::vector<std::vector<double>> vectors = IdentityMatrix(dimension);

  for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
    size_t p = 0;
    size_t q = 1;
    double max_off_diagonal = 0.0;

    for (size_t i = 0; i < dimension; ++i) {
      for (size_t j = i + 1; j < dimension; ++j) {
        const double entry_abs = std::fabs(a[i][j]);
        if (entry_abs > max_off_diagonal) {
          max_off_diagonal = entry_abs;
          p = i;
          q = j;
        }
      }
    }

    if (max_off_diagonal <= tolerance) {
      eigenvalues->assign(dimension, 0.0);
      for (size_t i = 0; i < dimension; ++i) {
        (*eigenvalues)[i] = a[i][i];
      }
      *eigenvectors = vectors;
      return true;
    }

    const double angle = 0.5 * std::atan2(2.0 * a[p][q], a[q][q] - a[p][p]);
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    const double app = a[p][p];
    const double aqq = a[q][q];
    const double apq = a[p][q];

    for (size_t k = 0; k < dimension; ++k) {
      if (k == p || k == q) {
        continue;
      }
      const double akp = a[k][p];
      const double akq = a[k][q];
      a[k][p] = c * akp - s * akq;
      a[p][k] = a[k][p];
      a[k][q] = s * akp + c * akq;
      a[q][k] = a[k][q];
    }

    a[p][p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
    a[q][q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
    a[p][q] = 0.0;
    a[q][p] = 0.0;

    for (size_t k = 0; k < dimension; ++k) {
      const double vkp = vectors[k][p];
      const double vkq = vectors[k][q];
      vectors[k][p] = c * vkp - s * vkq;
      vectors[k][q] = s * vkp + c * vkq;
    }
  }

  return false;
}

}  // namespace PricingHelper
