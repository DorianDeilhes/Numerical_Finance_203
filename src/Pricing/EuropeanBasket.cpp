#include "Pricing/EuropeanBasket.h"

#include "MonteCarlo/Helper/BasketCallPayoff.h"
#include "MonteCarlo/Helper/DiscountPayoff.h"
#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalND.h"
#include "Pricing/Helper/ValidateCorrelationMatrix.h"
#include <cmath>
#include <stdexcept>

EuropeanBasket::EuropeanBasket(const std::vector<double>& spot_prices,
                               const std::vector<double>& volatilities,
                               const std::vector<double>& weights,
                               double strike,
                               double maturity,
                               double risk_free_rate,
                               const std::vector<std::vector<double>>& correlation_matrix)
    : dimension_(spot_prices.size()),
      spot_prices_(spot_prices),
      volatilities_(volatilities),
      weights_(weights),
      strike_(strike),
      maturity_(maturity),
      risk_free_rate_(risk_free_rate),
      correlation_matrix_(correlation_matrix) {
  ValidateInputs();
}

void EuropeanBasket::ValidateInputs() {
  // Dimension consistency.
  if (dimension_ == 0) {
    throw std::invalid_argument("EuropeanBasket: dimension must be > 0");
  }
  if (volatilities_.size() != dimension_) {
    throw std::invalid_argument("EuropeanBasket: volatilities size mismatch");
  }
  if (weights_.size() != dimension_) {
    throw std::invalid_argument("EuropeanBasket: weights size mismatch");
  }

  // Parameter bounds.
  for (size_t i = 0; i < dimension_; ++i) {
    if (spot_prices_[i] <= 0.0) {
      throw std::invalid_argument("EuropeanBasket: spot prices must be positive");
    }
    if (volatilities_[i] < 0.0) {
      throw std::invalid_argument("EuropeanBasket: volatilities must be non-negative");
    }
  }
  if (strike_ < 0.0) {
    throw std::invalid_argument("EuropeanBasket: strike must be non-negative");
  }
  if (maturity_ <= 0.0) {
    throw std::invalid_argument("EuropeanBasket: maturity must be positive");
  }
  if (risk_free_rate_ < 0.0) {
    throw std::invalid_argument("EuropeanBasket: risk-free rate must be non-negative");
  }

  PricingHelper::ValidateCorrelationMatrix(correlation_matrix_, dimension_);
}

double EuropeanBasket::SinglePathPayoff(UniformGenerator* uniform_gen) {
  const size_t nbSteps = 100;
  std::vector<double> terminalSpots = MonteCarloHelper::SimulateGeometricBrownianTerminalND(
      uniform_gen, spot_prices_, volatilities_, risk_free_rate_, &correlation_matrix_, 0.0,
      maturity_, nbSteps);

  const double payoff = MonteCarloHelper::BasketCallPayoff(terminalSpots, weights_, strike_);
  return MonteCarloHelper::DiscountPayoff(payoff, risk_free_rate_, maturity_);
}

MonteCarloSummary EuropeanBasket::PriceFixedN(UniformGenerator* uniform_gen,
                                               size_t num_samples) {
  // Sampler: single path payoff (stateless, uses uniform_gen for randomness).
  const std::function<double()> sampler = [this, uniform_gen]() {
    return SinglePathPayoff(uniform_gen);
  };

  // Run Monte Carlo with fixed sample size.
  return MonteCarloCore::RunFixedN(sampler, num_samples, 0.95);
}

MonteCarloSummary EuropeanBasket::PriceFixedPrecision(UniformGenerator* uniform_gen,
                                                       double epsilon,
                                                       size_t min_samples,
                                                       size_t max_samples) {
  // Sampler: single path payoff.
  const std::function<double()> sampler = [this, uniform_gen]() {
    return SinglePathPayoff(uniform_gen);
  };

  // Run Monte Carlo with adaptive sample size to meet precision target.
  return MonteCarloCore::RunFixedPrecision(sampler, epsilon, 0.95, min_samples, max_samples);
}
