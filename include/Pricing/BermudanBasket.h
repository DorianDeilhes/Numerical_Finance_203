#ifndef BERMUDAN_BASKET_H
#define BERMUDAN_BASKET_H

#include "MonteCarlo/MonteCarloCore.h"
#include "UniformGenerator/UniformGenerator.h"

#include <utility>
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

  // Price with a static control variate estimated on pilot paths.
  MonteCarloSummary PriceFixedNControlVariate(UniformGenerator* uniform_gen,
                                              size_t path_count,
                                              size_t pilot_count = 500);

  // Price with antithetic paths and pair-averaged Longstaff-Schwarz values.
  MonteCarloSummary PriceFixedNAntithetic(UniformGenerator* uniform_gen,
                                          size_t pair_count);

  // Price with cumulative variance reduction: antithetic pairs plus static control variate.
  MonteCarloSummary PriceFixedNCumulative(UniformGenerator* uniform_gen,
                                          size_t pair_count,
                                          size_t pilot_count = 500);

 private:
  struct BasketStateSimulation {
    std::vector<std::vector<double>> basket_states_by_date;
    std::vector<double> geometric_controls;
  };

  size_t dimension_;
  std::vector<double> spot_prices_;
  std::vector<double> volatilities_;
  std::vector<double> weights_;
  double strike_;
  double maturity_;
  double risk_free_rate_;
  std::vector<std::vector<double>> correlation_matrix_;
  std::vector<std::vector<double>> loading_matrix_;
  std::vector<double> exercise_dates_;
  size_t nb_steps_;

  // Validate the contract parameters and exercise schedule.
  void ValidateInputs();

  // Simulate one path and compress its ND states into basket values at all exercise dates.
  std::vector<double> SimulateBasketStatesAtExerciseDates(UniformGenerator* uniform_gen);

  // Simulate one direct/antithetic pair and compress both paths into basket states.
  std::pair<std::vector<double>, std::vector<double>>
  SimulateBasketStatesAntitheticAtExerciseDates(UniformGenerator* uniform_gen);

  // Simulate many paths and store basket states as [exercise date][path].
  std::vector<std::vector<double>> SimulateBasketStatesByDate(UniformGenerator* uniform_gen,
                                                              size_t path_count);

  // Simulate antithetic pairs and store direct paths first, antithetic paths second.
  std::vector<std::vector<double>> SimulateAntitheticBasketStatesByDate(
      UniformGenerator* uniform_gen,
      size_t pair_count);

  // Simulate paths and keep both Longstaff-Schwarz states and geometric controls.
  BasketStateSimulation SimulateBasketStatesAndGeometricControlsByDate(
      UniformGenerator* uniform_gen,
      size_t path_count);

  // Simulate antithetic paths and keep geometric controls for direct and mirrored paths.
  BasketStateSimulation SimulateAntitheticBasketStatesAndGeometricControlsByDate(
      UniformGenerator* uniform_gen,
      size_t pair_count);

  // Apply Longstaff-Schwarz decisions and return discounted samples X_j.
  std::vector<double> ComputeDiscountedPathValues(
      const std::vector<std::vector<double>>& basket_states_by_date);

  // Build paired samples (X_j, Y_j) for the static control variate.
  std::pair<std::vector<double>, std::vector<double>> BuildPathValuesAndControls(
      UniformGenerator* uniform_gen,
      size_t path_count);

  // Build antithetic samples chi_j without constructing any control variate.
  std::vector<double> BuildAntitheticPairValues(UniformGenerator* uniform_gen,
                                                size_t pair_count);

  // Build paired antithetic samples (chi_j, Y_chi_j).
  std::pair<std::vector<double>, std::vector<double>> BuildAntitheticPairValuesAndControls(
      UniformGenerator* uniform_gen,
      size_t pair_count);

  // Summarize already-built samples through the shared MonteCarloCore.
  MonteCarloSummary SummarizeSamples(const std::vector<double>& samples);
};

#endif  // BERMUDAN_BASKET_H
