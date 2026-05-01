#include "Pricing/Helper/ValidateEuropeanBasketInputs.h"

#include "Pricing/Helper/ValidateCorrelationMatrix.h"

#include <cmath>
#include <stdexcept>

namespace PricingHelper {

void ValidateEuropeanBasketInputs(
    const std::vector<double>& spotPrices,
    const std::vector<double>& volatilities,
    const std::vector<double>& weights,
    double strike,
    double maturity,
    double riskFreeRate,
    const std::vector<std::vector<double>>& correlationMatrix,
    size_t nbSteps) {
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
    if (!std::isfinite(spotPrices[i])) {
      throw std::invalid_argument("EuropeanBasket: spot prices must be finite");
    }
    if (spotPrices[i] <= 0.0) {
      throw std::invalid_argument("EuropeanBasket: spot prices must be positive");
    }
    if (!std::isfinite(volatilities[i])) {
      throw std::invalid_argument("EuropeanBasket: volatilities must be finite");
    }
    if (volatilities[i] < 0.0) {
      throw std::invalid_argument("EuropeanBasket: volatilities must be non-negative");
    }
    if (!std::isfinite(weights[i])) {
      throw std::invalid_argument("EuropeanBasket: weights must be finite");
    }
  }

  if (!std::isfinite(strike)) {
    throw std::invalid_argument("EuropeanBasket: strike must be finite");
  }
  if (strike < 0.0) {
    throw std::invalid_argument("EuropeanBasket: strike must be non-negative");
  }
  if (!std::isfinite(maturity)) {
    throw std::invalid_argument("EuropeanBasket: maturity must be finite");
  }
  if (maturity <= 0.0) {
    throw std::invalid_argument("EuropeanBasket: maturity must be positive");
  }
  if (!std::isfinite(riskFreeRate)) {
    throw std::invalid_argument("EuropeanBasket: risk-free rate must be finite");
  }
  if (riskFreeRate < 0.0) {
    throw std::invalid_argument("EuropeanBasket: risk-free rate must be non-negative");
  }
  if (nbSteps == 0) {
    throw std::invalid_argument("EuropeanBasket: nb_steps must be > 0");
  }

  ValidateCorrelationMatrix(correlationMatrix, dimension);
}

}  // namespace PricingHelper
