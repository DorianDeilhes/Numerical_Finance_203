#include "Pricing/Helper/ValidateCorrelationMatrix.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

void ValidateCorrelationMatrix(const std::vector<std::vector<double>>& correlationMatrix,
                               size_t dimension) {
  if (dimension == 0) {
    throw std::invalid_argument("Pricing: correlation_matrix dimension must be > 0");
  }
  if (correlationMatrix.size() != dimension) {
    throw std::invalid_argument("Pricing: correlation_matrix rows mismatch");
  }
  for (const auto& row : correlationMatrix) {
    if (row.size() != dimension) {
      throw std::invalid_argument("Pricing: correlation_matrix columns mismatch");
    }
  }

  for (size_t i = 0; i < dimension; ++i) {
    for (size_t j = 0; j < dimension; ++j) {
      if (!std::isfinite(correlationMatrix[i][j])) {
        throw std::invalid_argument("Pricing: correlation matrix entries must be finite");
      }
    }
    if (std::fabs(correlationMatrix[i][i] - 1.0) > 1e-10) {
      throw std::invalid_argument("Pricing: correlation matrix diagonal must be 1");
    }
    for (size_t j = i + 1; j < dimension; ++j) {
      if (std::fabs(correlationMatrix[i][j] - correlationMatrix[j][i]) > 1e-10) {
        throw std::invalid_argument("Pricing: correlation matrix must be symmetric");
      }
      if (correlationMatrix[i][j] < -1.0 || correlationMatrix[i][j] > 1.0 ||
          correlationMatrix[j][i] < -1.0 || correlationMatrix[j][i] > 1.0) {
        throw std::invalid_argument("Pricing: correlation entries must be in [-1,1]");
      }
    }
  }
}

}  // namespace PricingHelper
