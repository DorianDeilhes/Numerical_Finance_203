#include "Pricing/Helper/TryCholeskyDecomposition.h"

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

}  // namespace

namespace PricingHelper {

bool TryCholeskyDecomposition(const std::vector<std::vector<double>>& matrix,
                              std::vector<std::vector<double>>* lower) {
  if (lower == nullptr) {
    throw std::runtime_error("TryCholeskyDecomposition requires non-null output");
  }
  ValidateSquareFiniteMatrix(matrix, "TryCholeskyDecomposition");

  const size_t dimension = matrix.size();
  const double tolerance = 1e-12;
  lower->assign(dimension, std::vector<double>(dimension, 0.0));

  for (size_t i = 0; i < dimension; ++i) {
    for (size_t j = 0; j <= i; ++j) {
      double sum = matrix[i][j];
      for (size_t k = 0; k < j; ++k) {
        sum -= (*lower)[i][k] * (*lower)[j][k];
      }

      if (i == j) {
        if (sum <= tolerance) {
          lower->clear();
          return false;
        }
        (*lower)[i][j] = std::sqrt(sum);
      } else {
        if (std::fabs((*lower)[j][j]) <= tolerance) {
          lower->clear();
          return false;
        }
        (*lower)[i][j] = sum / (*lower)[j][j];
      }
    }
  }

  return true;
}

}  // namespace PricingHelper
