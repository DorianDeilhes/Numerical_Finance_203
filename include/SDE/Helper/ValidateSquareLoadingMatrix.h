#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace SDEHelper {

void ValidateSquareLoadingMatrix(std::vector<std::vector<double>>* loadingMatrix,
                                 size_t dimension,
                                 const std::string& functionName);

}  // namespace SDEHelper