#include "Pricing/Helper/ValidateBermudanBasketInputs.h"

#include "Pricing/Helper/ValidateCorrelationMatrix.h"
#include "Pricing/Helper/ValidateExerciseDates.h"

#include <stdexcept>

namespace PricingHelper {

void ValidateBermudanBasketInputs(const std::vector<double>& spot_prices,
                                  const std::vector<double>& volatilities,
                                  const std::vector<double>& weights,
                                  double strike,
                                  double maturity,
                                  double risk_free_rate,
                                  const std::vector<std::vector<double>>& correlation_matrix,
                                  const std::vector<double>& exercise_dates,
                                  size_t nb_steps) {
  // Basic dimension and positivity checks for market/payoff inputs.
  const size_t dimension = spot_prices.size();
  if (dimension == 0) {
    throw std::invalid_argument("BermudanBasket: dimension must be > 0");
  }
  if (volatilities.size() != dimension) {
    throw std::invalid_argument("BermudanBasket: volatilities size mismatch");
  }
  if (weights.size() != dimension) {
    throw std::invalid_argument("BermudanBasket: weights size mismatch");
  }

  for (size_t i = 0; i < dimension; ++i) {
    if (spot_prices[i] <= 0.0) {
      throw std::invalid_argument("BermudanBasket: spot prices must be positive");
    }
    if (volatilities[i] < 0.0) {
      throw std::invalid_argument("BermudanBasket: volatilities must be non-negative");
    }
  }

  if (strike < 0.0) {
    throw std::invalid_argument("BermudanBasket: strike must be non-negative");
  }
  if (maturity <= 0.0) {
    throw std::invalid_argument("BermudanBasket: maturity must be positive");
  }
  if (risk_free_rate < 0.0) {
    throw std::invalid_argument("BermudanBasket: risk-free rate must be non-negative");
  }
  if (nb_steps == 0) {
    throw std::invalid_argument("BermudanBasket: nb_steps must be > 0");
  }

  // Structural checks are delegated to dedicated helpers.
  ValidateCorrelationMatrix(correlation_matrix, dimension);
  ValidateExerciseDates(exercise_dates, maturity);
}

}  // namespace PricingHelper
