#include "Pricing/Helper/ValidateEuropeanBasketInputs.h"

#include "Pricing/Helper/ValidateCorrelationMatrix.h"

#include <stdexcept>

namespace PricingHelper {

void ValidateEuropeanBasketInputs(
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    const std::vector<double>& weights,
    double strike,
    double maturity,
    double riskFreeRate,
    const std::vector<std::vector<double>>& correlationMatrix) {
  const size_t dimension = spotPrices.size();

  if (dimension == 0) {
    throw std::invalid_argument("EuropeanBasket: dimension must be > 0");
  }
  if (volatilities.size() != dimension) {
    throw std::invalid_argument("EuropeanBasket: volatilities size mismatch");
  }
  if (weights.size() != dimension) {
    throw std::invalid_argument("EuropeanBasket: weights size mismatch");
  }

  for (size_t i = 0; i < dimension; ++i) {
    if (spotPrices[i] <= 0.0) {
      throw std::invalid_argument("EuropeanBasket: spot prices must be positive");
    }
    if (volatilities[i] < 0.0) {
      throw std::invalid_argument("EuropeanBasket: volatilities must be non-negative");
    }
  }

  if (strike < 0.0) {
    throw std::invalid_argument("EuropeanBasket: strike must be non-negative");
  }
  if (maturity <= 0.0) {
    throw std::invalid_argument("EuropeanBasket: maturity must be positive");
  }
  if (riskFreeRate < 0.0) {
    throw std::invalid_argument("EuropeanBasket: risk-free rate must be non-negative");
  }

  ValidateCorrelationMatrix(correlationMatrix, dimension);
}

}  // namespace PricingHelper
