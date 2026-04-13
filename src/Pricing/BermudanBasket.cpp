#include "Pricing/BermudanBasket.h"

#include "MonteCarlo/Helper/SimulateGeometricBrownianSpotsAtStepIndicesND.h"
#include "Pricing/Helper/BasketValue.h"
#include "Pricing/Helper/BermudanImmediateExerciseCall.h"
#include "Pricing/Helper/ConvertExerciseDatesToStepIndices.h"
#include "Pricing/Helper/EstimateQuadraticContinuationCoefficients.h"
#include "Pricing/Helper/EvaluateQuadraticContinuation.h"
#include "Pricing/Helper/ValidateBermudanBasketInputs.h"

#include <array>
#include <cmath>
#include <functional>
#include <stdexcept>

BermudanBasket::BermudanBasket(const std::vector<double>& spot_prices,
                               const std::vector<double>& volatilities,
                               const std::vector<double>& weights,
                               double strike,
                               double maturity,
                               double risk_free_rate,
                               const std::vector<std::vector<double>>& correlation_matrix,
                               const std::vector<double>& exercise_dates,
                               size_t nb_steps)
    : dimension_(spot_prices.size()),
      spot_prices_(spot_prices),
      volatilities_(volatilities),
      weights_(weights),
      strike_(strike),
      maturity_(maturity),
      risk_free_rate_(risk_free_rate),
      correlation_matrix_(correlation_matrix),
      exercise_dates_(exercise_dates),
      nb_steps_(nb_steps) {
  ValidateInputs();
}

void BermudanBasket::ValidateInputs() {
  PricingHelper::ValidateBermudanBasketInputs(spot_prices_, volatilities_, weights_, strike_,
                                               maturity_, risk_free_rate_, correlation_matrix_,
                                               exercise_dates_, nb_steps_);
}

std::vector<double> BermudanBasket::SimulateBasketStatesAtExerciseDates(
    UniformGenerator* uniform_gen) {
  // Map contractual exercise times to simulation-grid step indices.
  const std::vector<size_t> step_indices =
      PricingHelper::ConvertExerciseDatesToStepIndices(exercise_dates_, maturity_, nb_steps_);

  // Simulate one ND path and retain spots only at requested exercise steps.
  const std::vector<std::vector<double>> spots_by_exercise =
      MonteCarloHelper::SimulateGeometricBrownianSpotsAtStepIndicesND(
          uniform_gen, spot_prices_, volatilities_, risk_free_rate_, &correlation_matrix_, 0.0,
          maturity_, nb_steps_, step_indices);

  // Collapse ND spot vectors to a scalar basket state per exercise date.
  std::vector<double> basket_states;
  basket_states.reserve(spots_by_exercise.size());
  for (size_t k = 0; k < spots_by_exercise.size(); ++k) {
    basket_states.push_back(PricingHelper::BasketValue(spots_by_exercise[k], weights_));
  }
  return basket_states;
}

MonteCarloSummary BermudanBasket::PriceFixedN(UniformGenerator* uniform_gen,
                                              size_t path_count) {
  // Stage A: input guards.
  if (uniform_gen == nullptr) {
    throw std::runtime_error("BermudanBasket::PriceFixedN requires a non-null uniform generator");
  }
  if (path_count < 2) {
    throw std::runtime_error("BermudanBasket::PriceFixedN requires path_count >= 2");
  }

  const size_t exercise_count = exercise_dates_.size();
  std::vector<std::vector<double>> basket_states_by_date(
      exercise_count, std::vector<double>(path_count, 0.0));

  // Stage B: forward simulation cache (basket state at every exercise date for every path).
  for (size_t path = 0; path < path_count; ++path) {
    const std::vector<double> basket_states = SimulateBasketStatesAtExerciseDates(uniform_gen);
    for (size_t k = 0; k < exercise_count; ++k) {
      basket_states_by_date[k][path] = basket_states[k];
    }
  }

  // Stage C: terminal condition at maturity.
  const size_t maturity_index = exercise_count - 1;
  std::vector<double> values_next(path_count, 0.0);
  for (size_t path = 0; path < path_count; ++path) {
    values_next[path] = PricingHelper::BermudanImmediateExerciseCall(
        basket_states_by_date[maturity_index][path], strike_);
  }

  // Stage D: Longstaff-Schwarz backward induction over exercise dates.
  for (size_t k = exercise_count - 1; k-- > 0;) {
    const double dt = exercise_dates_[k + 1] - exercise_dates_[k];
    const double discount_factor = std::exp(-risk_free_rate_ * dt);

    std::vector<double> states_itm;
    std::vector<double> targets_itm;
    states_itm.reserve(path_count);
    targets_itm.reserve(path_count);

    for (size_t path = 0; path < path_count; ++path) {
      const double basket_state = basket_states_by_date[k][path];
      const double immediate = PricingHelper::BermudanImmediateExerciseCall(basket_state, strike_);
      if (immediate > 0.0) {
        states_itm.push_back(basket_state);
        targets_itm.push_back(discount_factor * values_next[path]);
      }
    }

    // Regress discounted continuation values on quadratic basis (1, B, B^2).
    std::array<double, 3> coefficients = {{0.0, 0.0, 0.0}};
    const bool has_regression = PricingHelper::EstimateQuadraticContinuationCoefficients(
        states_itm, targets_itm, &coefficients);

    std::vector<double> values_current(path_count, 0.0);
    for (size_t path = 0; path < path_count; ++path) {
      const double basket_state = basket_states_by_date[k][path];
      const double immediate = PricingHelper::BermudanImmediateExerciseCall(basket_state, strike_);
      const double continuation_realized = discount_factor * values_next[path];

      if (immediate <= 0.0 || !has_regression) {
        values_current[path] = continuation_realized;
        continue;
      }

      const double continuation_estimated =
          PricingHelper::EvaluateQuadraticContinuation(basket_state, coefficients);
        // Optimal Bermudan decision at t_k: exercise now or continue.
        values_current[path] =
          (immediate >= continuation_estimated) ? immediate : continuation_realized;
    }

    values_next.swap(values_current);
  }

      // Stage E: discount to time 0 and summarize with Monte Carlo statistics.
  const double first_discount = std::exp(-risk_free_rate_ * exercise_dates_.front());
  std::vector<double> discounted_path_values(path_count, 0.0);
  for (size_t path = 0; path < path_count; ++path) {
    discounted_path_values[path] = first_discount * values_next[path];
  }

  size_t cursor = 0;
  const std::function<double()> sampler = [&discounted_path_values, &cursor]() {
    return discounted_path_values[cursor++];
  };

  return MonteCarloCore::RunFixedN(sampler, path_count, 0.95);
}
