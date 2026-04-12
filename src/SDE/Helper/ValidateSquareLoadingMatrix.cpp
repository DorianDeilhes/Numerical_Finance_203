#include "SDE/Helper/ValidateSquareLoadingMatrix.h"

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
  for (const std::vector<double>& row : *loadingMatrix) {
    if (row.size() != dimension) {
      throw std::runtime_error(functionName + " requires a square loading matrix matching the process dimension");
    }
  }
}

}  // namespace SDEHelper