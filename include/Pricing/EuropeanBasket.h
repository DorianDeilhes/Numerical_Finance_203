#ifndef EUROPEAN_BASKET_H
#define EUROPEAN_BASKET_H

#include "MonteCarlo/MonteCarloCore.h"
#include "UniformGenerator/UniformGenerator.h"

#include <functional>
#include <vector>

// European basket option under multidimensional Black-Scholes.
// Payoff: max(sum(w_i * S_i(T)) - K, 0) at maturity T.
// Each asset S_i evolves under dS_i = r S_i dt + sigma_i S_i dW_i,
// where dW is correlated via the provided correlation matrix.
class EuropeanBasket {
 public:
  // Constructor validates all inputs (dimension consistency, positive parameters, correlation bounds).
  EuropeanBasket(const std::vector<double>& spot_prices,
                 const std::vector<double>& volatilities,
                 const std::vector<double>& weights,
                 double strike,
                 double maturity,
                 double risk_free_rate,
                 const std::vector<std::vector<double>>& correlation_matrix);

  // Price using fixed sample size N with 95% confidence interval.
  MonteCarloSummary PriceFixedN(UniformGenerator* uniform_gen, size_t num_samples);

  // Price using fixed precision epsilon, adapting sample count.
  MonteCarloSummary PriceFixedPrecision(UniformGenerator* uniform_gen,
                                        double epsilon,
                                        size_t min_samples = 100,
                                        size_t max_samples = 200000);

 private:
  size_t dimension_;
  std::vector<double> spot_prices_;
  std::vector<double> volatilities_;
  std::vector<double> weights_;
  double strike_;
  double maturity_;
  double risk_free_rate_;
  std::vector<std::vector<double>> correlation_matrix_;

  // Single path payoff: simulate one trajectory, return discounted payoff.
  double SinglePathPayoff(UniformGenerator* uniform_gen);

  // Validate constructor inputs.
  void ValidateInputs();
};

#endif  // EUROPEAN_BASKET_H
