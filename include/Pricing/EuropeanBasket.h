#ifndef EUROPEAN_BASKET_H
#define EUROPEAN_BASKET_H

#include "MonteCarlo/MonteCarloCore.h"
#include "UniformGenerator/UniformGenerator.h"

#include <functional>
#include <utility>
#include <vector>

// European basket option under multidimensional Black-Scholes.
// Payoff: max(sum(w_i * S_i(T)) - K, 0) at maturity T.
// Each asset S_i evolves under dS_i = r S_i dt + sigma_i S_i dW_i,
// where dW is correlated via the provided correlation matrix.
class EuropeanBasket {
 public:
  // Constructor validates all inputs (dimension consistency, positive parameters, correlation bounds).
  // nb_steps: number of time steps for SDE discretization (Euler scheme). Default: 100.
  // Higher values improve accuracy but increase computational cost.
  EuropeanBasket(const std::vector<double>& spot_prices,
                 const std::vector<double>& volatilities,
                 const std::vector<double>& weights,
                 double strike,
                 double maturity,
                 double risk_free_rate,
                 const std::vector<std::vector<double>>& correlation_matrix,
                 size_t nb_steps = 100);

  // Price using fixed sample size N with 95% confidence interval.
  MonteCarloSummary PriceFixedN(UniformGenerator* uniform_gen, size_t num_samples);

  // Price using fixed precision epsilon, adapting sample count.
  MonteCarloSummary PriceFixedPrecision(UniformGenerator* uniform_gen,
                                        double epsilon,
                                        size_t min_samples = 100,
                                        size_t max_samples = 200000);

  // Fixed-N antithetic pricing using pair-averaged payoffs.
  MonteCarloSummary PriceFixedNAntithetic(UniformGenerator* uniform_gen,
                                          size_t pair_count);

  // Fixed-N control variate pricing with static beta from pilot samples.
  MonteCarloSummary PriceFixedNControlVariate(UniformGenerator* uniform_gen,
                                              size_t sample_count,
                                              size_t pilot_count = 500);

  // Fixed-N cumulative mode: antithetic + static control variate.
  MonteCarloSummary PriceFixedNCumulative(UniformGenerator* uniform_gen,
                                          size_t pair_count,
                                          size_t pilot_count = 500);

 private:
  size_t dimension_;
  std::vector<double> spot_prices_;
  std::vector<double> volatilities_;
  std::vector<double> weights_;
  double strike_;
  double maturity_;
  double risk_free_rate_;
  std::vector<std::vector<double>> correlation_matrix_;
  std::vector<std::vector<double>> loading_matrix_;
  size_t nb_steps_;  // Number of time steps for SDE discretization.

  // Single path payoff: simulate one trajectory, return discounted payoff.
  double SinglePathPayoff(UniformGenerator* uniform_gen);

  // Single-path control variate pair using the lecture geometric basket control.
  std::pair<double, double> SinglePathPayoffAndControl(UniformGenerator* uniform_gen);

  // Antithetic pair of target and geometric control samples.
  std::pair<double, double> SinglePathAntitheticAverages(UniformGenerator* uniform_gen);

  // Antithetic payoff only (optimized for antithetic-only modes, no control computation).
  double SinglePathAntitheticPayoffOnly(UniformGenerator* uniform_gen);

  // Validate constructor inputs.
  void ValidateInputs();
};

#endif  // EUROPEAN_BASKET_H
