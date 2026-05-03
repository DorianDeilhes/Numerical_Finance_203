#include "SDE/Helper/ValidateSquareLoadingMatrix.h"

#include <cmath>
#include <stdexcept>

namespace SDEHelper {

void ValidateSquareLoadingMatrix(std::vector<std::vector<double>>* loadingMatrix,
                                 size_t dimension,
                                 const std::string& functionName) {
  if (loadingMatrix == nullptr) {
    return;
  }

  if (loadingMatrix->size() != dimension) {
    throw std::runtime_error(functionName + " requires a square loading matrix matching the process dimension");
  }
  for (size_t i = 0; i < loadingMatrix->size(); ++i) {
    const std::vector<double>& row = (*loadingMatrix)[i];
    if (row.size() != dimension) {
      throw std::runtime_error(functionName + " requires a square loading matrix matching the process dimension");
    }
    for (size_t j = 0; j < row.size(); ++j) {
      if (!std::isfinite(row[j])) {
        throw std::runtime_error(functionName + " requires finite loading matrix entries");
      }
    }
  }
}

}  // namespace SDEHelper
