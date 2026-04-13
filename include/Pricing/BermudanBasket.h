#ifndef BERMUDAN_BASKET_H
#define BERMUDAN_BASKET_H

#include "MonteCarlo/MonteCarloCore.h"
#include "UniformGenerator/UniformGenerator.h"

#include <vector>

// Bermudan basket call option under multidimensional Black-Scholes.
// Pricing is done by Longstaff-Schwarz backward induction on a fixed exercise grid.
class BermudanBasket {
 public:
  // Build a Bermudan basket pricer with an explicit exercise schedule and a configurable time grid.
  BermudanBasket(const std::vector<double>& spot_prices,
                 const std::vector<double>& volatilities,
                 const std::vector<double>& weights,
                 double strike,
                 double maturity,
                 double risk_free_rate,
                 const std::vector<std::vector<double>>& correlation_matrix,
                 const std::vector<double>& exercise_dates,
                 size_t nb_steps = 100);

  // Price the Bermudan contract with fixed-path Monte Carlo and regression-based exercise decisions.
  MonteCarloSummary PriceFixedN(UniformGenerator* uniform_gen, size_t path_count);

 private:
  size_t dimension_;
  std::vector<double> spot_prices_;
  std::vector<double> volatilities_;
  std::vector<double> weights_;
  double strike_;
  double maturity_;
  double risk_free_rate_;
  std::vector<std::vector<double>> correlation_matrix_;
  std::vector<double> exercise_dates_;
  size_t nb_steps_;

  // Validate the contract parameters and exercise schedule.
  void ValidateInputs();

  // Simulate one path and compress its ND states into basket values at all exercise dates.
  std::vector<double> SimulateBasketStatesAtExerciseDates(UniformGenerator* uniform_gen);
};

#endif  // BERMUDAN_BASKET_H
