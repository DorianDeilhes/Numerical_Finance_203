#include "Pricing/Helper/EstimateControlVariateBeta.h"

#include <stdexcept>

namespace PricingHelper {

double EstimateControlVariateBeta(const std::vector<double>& targetSamples,
                                  const std::vector<double>& controlSamples) {
  if (targetSamples.size() != controlSamples.size()) {
    throw std::runtime_error(
        "EstimateControlVariateBeta requires matching sample sizes");
  }
  if (targetSamples.size() < 2) {
    throw std::runtime_error(
        "EstimateControlVariateBeta requires at least two pilot samples");
  }

  const size_t n = targetSamples.size();

  double meanX = 0.0;
  double meanY = 0.0;
  for (size_t i = 0; i < n; ++i) {
    meanX += targetSamples[i];
    meanY += controlSamples[i];
  }
  meanX /= static_cast<double>(n);
  meanY /= static_cast<double>(n);

  double covariance = 0.0;
  double varianceY = 0.0;
  for (size_t i = 0; i < n; ++i) {
    const double dx = targetSamples[i] - meanX;
    const double dy = controlSamples[i] - meanY;
    covariance += dx * dy;
    varianceY += dy * dy;
  }

  if (varianceY <= 0.0) {
    return 0.0;
  }

  return covariance / varianceY;
}

}  // namespace PricingHelper
