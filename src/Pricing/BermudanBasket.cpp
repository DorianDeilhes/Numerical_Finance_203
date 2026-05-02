// Design note for the final report:
// The Longstaff-Schwarz baseline uses a fixed quadratic basis in the scalar basket
// state B: (1, B, B^2). A natural extension would be a configurable basis
// (higher-order polynomials, payoff-based terms, or asset-level states S_i).
// This was intentionally left out to keep the Bermudan implementation basic,
// transparent, and aligned with the lecture project scope.

#include "Pricing/BermudanBasket.h"

#include "MonteCarlo/Helper/SimulateGeometricBrownianSpotsAtStepIndicesND.h"
#include "MonteCarlo/Helper/SimulateGeometricBrownianSpotsAtStepIndicesNDAntithetic.h"
#include "Pricing/Helper/ApplyControlVariate.h"
#include "Pricing/Helper/BasketValue.h"
#include "Pricing/Helper/BermudanImmediateExerciseCall.h"
#include "Pricing/Helper/BuildLoadingMatrixFromCorrelation.h"
#include "Pricing/Helper/ConvertExerciseDatesToStepIndices.h"
#include "Pricing/Helper/DiscountedGeometricBasketCall.h"
#include "Pricing/Helper/EstimateControlVariateBeta.h"
#include "Pricing/Helper/EstimateQuadraticContinuationCoefficients.h"
#include "Pricing/Helper/EvaluateQuadraticContinuation.h"
#include "Pricing/Helper/KnownMeanDiscountedGeometricBasketCall.h"
#include "Pricing/Helper/ValidateBermudanBasketInputs.h"
#include "Pricing/Helper/ValidatePricingMethodInputs.h"

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
                               size_t nb_steps,
                               const std::vector<double>& dividend_yields)
    : dimension_(spot_prices.size()),
      spot_prices_(spot_prices),
      volatilities_(volatilities),
      weights_(weights),
      strike_(strike),
      maturity_(maturity),
      risk_free_rate_(risk_free_rate),
      dividend_yields_(dividend_yields.empty()
                           ? std::vector<double>(spot_prices.size(), 0.0)
                           : dividend_yields),
      correlation_matrix_(correlation_matrix),
      loading_matrix_(),
      exercise_dates_(exercise_dates),
      nb_steps_(nb_steps) {
  ValidateInputs();
  loading_matrix_ = PricingHelper::BuildLoadingMatrixFromCorrelation(correlation_matrix_);
}

void BermudanBasket::ValidateInputs() {
  PricingHelper::ValidateBermudanBasketInputs(spot_prices_, volatilities_, weights_, strike_,
                                               maturity_, risk_free_rate_, correlation_matrix_,
                                               exercise_dates_, dividend_yields_, nb_steps_);
}

std::vector<double> BermudanBasket::SimulateBasketStatesAtExerciseDates(
    UniformGenerator* uniform_gen) {
  // Map contractual exercise times to simulation-grid step indices.
  const std::vector<size_t> step_indices =
      PricingHelper::ConvertExerciseDatesToStepIndices(exercise_dates_, maturity_, nb_steps_);

  // Simulate one ND path and retain spots only at requested exercise steps.
  const std::vector<std::vector<double>> spots_by_exercise =
      MonteCarloHelper::SimulateGeometricBrownianSpotsAtStepIndicesND(
          uniform_gen, spot_prices_, volatilities_, risk_free_rate_, dividend_yields_,
          &loading_matrix_, 0.0, maturity_, nb_steps_, step_indices);

  // Collapse ND spot vectors to a scalar basket state per exercise date.
  std::vector<double> basket_states;
  basket_states.reserve(spots_by_exercise.size());
  for (size_t k = 0; k < spots_by_exercise.size(); ++k) {
    basket_states.push_back(PricingHelper::BasketValue(spots_by_exercise[k], weights_));
  }
  return basket_states;
}

std::pair<std::vector<double>, std::vector<double>>
BermudanBasket::SimulateBasketStatesAntitheticAtExerciseDates(UniformGenerator* uniform_gen) {
  const std::vector<size_t> step_indices =
      PricingHelper::ConvertExerciseDatesToStepIndices(exercise_dates_, maturity_, nb_steps_);

  const MonteCarloHelper::ExerciseSpotsAntitheticPair spots_by_exercise =
      MonteCarloHelper::SimulateGeometricBrownianSpotsAtStepIndicesNDAntithetic(
          uniform_gen, spot_prices_, volatilities_, risk_free_rate_, dividend_yields_,
          &loading_matrix_, 0.0, maturity_, nb_steps_, step_indices);

  std::vector<double> direct_basket_states;
  std::vector<double> antithetic_basket_states;
  direct_basket_states.reserve(spots_by_exercise.direct.size());
  antithetic_basket_states.reserve(spots_by_exercise.antithetic.size());

  for (size_t k = 0; k < spots_by_exercise.direct.size(); ++k) {
    direct_basket_states.push_back(PricingHelper::BasketValue(spots_by_exercise.direct[k],
                                                              weights_));
    antithetic_basket_states.push_back(PricingHelper::BasketValue(
        spots_by_exercise.antithetic[k], weights_));
  }

  return std::make_pair(direct_basket_states, antithetic_basket_states);
}

std::vector<std::vector<double>> BermudanBasket::SimulateBasketStatesByDate(
    UniformGenerator* uniform_gen,
    size_t path_count) {
  const size_t exercise_count = exercise_dates_.size();
  std::vector<std::vector<double>> basket_states_by_date(
      exercise_count, std::vector<double>(path_count, 0.0));

  for (size_t path = 0; path < path_count; ++path) {
    const std::vector<double> basket_states = SimulateBasketStatesAtExerciseDates(uniform_gen);
    for (size_t k = 0; k < exercise_count; ++k) {
      basket_states_by_date[k][path] = basket_states[k];
    }
  }

  return basket_states_by_date;
}

std::vector<std::vector<double>> BermudanBasket::SimulateAntitheticBasketStatesByDate(
    UniformGenerator* uniform_gen,
    size_t pair_count) {
  const size_t exercise_count = exercise_dates_.size();
  std::vector<std::vector<double>> basket_states_by_date(
      exercise_count, std::vector<double>(2 * pair_count, 0.0));

  for (size_t j = 0; j < pair_count; ++j) {
    const std::pair<std::vector<double>, std::vector<double>> basket_pair =
        SimulateBasketStatesAntitheticAtExerciseDates(uniform_gen);

    for (size_t k = 0; k < exercise_count; ++k) {
      basket_states_by_date[k][j] = basket_pair.first[k];
      basket_states_by_date[k][pair_count + j] = basket_pair.second[k];
    }
  }

  return basket_states_by_date;
}

BermudanBasket::BasketStateSimulation
BermudanBasket::SimulateBasketStatesAndGeometricControlsByDate(UniformGenerator* uniform_gen,
                                                               size_t path_count) {
  const size_t exercise_count = exercise_dates_.size();
  BasketStateSimulation simulation;
  simulation.basket_states_by_date.assign(
      exercise_count, std::vector<double>(path_count, 0.0));
  simulation.geometric_controls.assign(path_count, 0.0);

  const std::vector<size_t> step_indices =
      PricingHelper::ConvertExerciseDatesToStepIndices(exercise_dates_, maturity_, nb_steps_);

  for (size_t path = 0; path < path_count; ++path) {
    const std::vector<std::vector<double>> spots_by_exercise =
        MonteCarloHelper::SimulateGeometricBrownianSpotsAtStepIndicesND(
            uniform_gen, spot_prices_, volatilities_, risk_free_rate_, dividend_yields_,
            &loading_matrix_, 0.0, maturity_, nb_steps_, step_indices);

    for (size_t k = 0; k < exercise_count; ++k) {
      simulation.basket_states_by_date[k][path] =
          PricingHelper::BasketValue(spots_by_exercise[k], weights_);
    }

    simulation.geometric_controls[path] = PricingHelper::DiscountedGeometricBasketCall(
        spots_by_exercise.back(), weights_, strike_, risk_free_rate_, maturity_);
  }

  return simulation;
}

BermudanBasket::BasketStateSimulation
BermudanBasket::SimulateAntitheticBasketStatesAndGeometricControlsByDate(
    UniformGenerator* uniform_gen,
    size_t pair_count) {
  const size_t exercise_count = exercise_dates_.size();
  BasketStateSimulation simulation;
  simulation.basket_states_by_date.assign(
      exercise_count, std::vector<double>(2 * pair_count, 0.0));
  simulation.geometric_controls.assign(2 * pair_count, 0.0);

  const std::vector<size_t> step_indices =
      PricingHelper::ConvertExerciseDatesToStepIndices(exercise_dates_, maturity_, nb_steps_);

  for (size_t j = 0; j < pair_count; ++j) {
    const MonteCarloHelper::ExerciseSpotsAntitheticPair spots_by_exercise =
        MonteCarloHelper::SimulateGeometricBrownianSpotsAtStepIndicesNDAntithetic(
            uniform_gen, spot_prices_, volatilities_, risk_free_rate_, dividend_yields_,
            &loading_matrix_, 0.0, maturity_, nb_steps_, step_indices);

    for (size_t k = 0; k < exercise_count; ++k) {
      simulation.basket_states_by_date[k][j] =
          PricingHelper::BasketValue(spots_by_exercise.direct[k], weights_);
      simulation.basket_states_by_date[k][pair_count + j] =
          PricingHelper::BasketValue(spots_by_exercise.antithetic[k], weights_);
    }

    simulation.geometric_controls[j] = PricingHelper::DiscountedGeometricBasketCall(
        spots_by_exercise.direct.back(), weights_, strike_, risk_free_rate_, maturity_);
    simulation.geometric_controls[pair_count + j] = PricingHelper::DiscountedGeometricBasketCall(
        spots_by_exercise.antithetic.back(), weights_, strike_, risk_free_rate_, maturity_);
  }

  return simulation;
}

std::vector<double> BermudanBasket::ComputeDiscountedPathValues(
    const std::vector<std::vector<double>>& basket_states_by_date) {
  const size_t exercise_count = exercise_dates_.size();
  if (basket_states_by_date.size() != exercise_count) {
    throw std::runtime_error(
        "BermudanBasket::ComputeDiscountedPathValues requires one row per exercise date");
  }
  if (basket_states_by_date.empty() || basket_states_by_date[0].empty()) {
    throw std::runtime_error(
        "BermudanBasket::ComputeDiscountedPathValues requires non-empty path states");
  }

  const size_t path_count = basket_states_by_date[0].size();
  for (size_t k = 1; k < basket_states_by_date.size(); ++k) {
    if (basket_states_by_date[k].size() != path_count) {
      throw std::runtime_error(
          "BermudanBasket::ComputeDiscountedPathValues requires rectangular path states");
    }
  }

  // Terminal condition: V_T = f(S_T).
  const size_t maturity_index = exercise_count - 1;
  std::vector<double> values_next(path_count, 0.0);
  for (size_t path = 0; path < path_count; ++path) {
    values_next[path] = PricingHelper::BermudanImmediateExerciseCall(
        basket_states_by_date[maturity_index][path], strike_);
  }

  // Longstaff-Schwarz backward induction over exercise dates.
  for (size_t k = exercise_count - 1; k-- > 0;) {
    const double dt = exercise_dates_[k + 1] - exercise_dates_[k];
    const double discount_factor = std::exp(-risk_free_rate_ * dt);

    std::vector<double> states_itm;
    std::vector<double> targets_itm;
    states_itm.reserve(path_count);
    targets_itm.reserve(path_count);

    for (size_t j = 0; j < path_count; ++j) {
      const double basket_state = basket_states_by_date[k][j];
      const double immediate = PricingHelper::BermudanImmediateExerciseCall(basket_state, strike_);
      if (immediate > 0.0) {
        states_itm.push_back(basket_state);
        targets_itm.push_back(discount_factor * values_next[j]);
      }
    }

    // Regress discounted continuation values on quadratic basis (1, B, B^2).
    std::array<double, 3> coefficients = {{0.0, 0.0, 0.0}};
    const bool has_regression = PricingHelper::EstimateQuadraticContinuationCoefficients(
        states_itm, targets_itm, &coefficients);

    double fallback_continuation = 0.0;
    const bool has_fallback_continuation = !targets_itm.empty();
    if (has_fallback_continuation) {
      for (size_t i = 0; i < targets_itm.size(); ++i) {
        fallback_continuation += targets_itm[i];
      }
      fallback_continuation /= static_cast<double>(targets_itm.size());
    }

    std::vector<double> values_current(path_count, 0.0);
    for (size_t j = 0; j < path_count; ++j) {
      const double basket_state = basket_states_by_date[k][j];
      const double immediate = PricingHelper::BermudanImmediateExerciseCall(basket_state, strike_);
      const double continuation_realized = discount_factor * values_next[j];

      if (immediate <= 0.0) {
        values_current[j] = continuation_realized;
        continue;
      }

      if (!has_regression && !has_fallback_continuation) {
        values_current[j] = continuation_realized;
        continue;
      }

      const double continuation_estimated = has_regression
          ? PricingHelper::EvaluateQuadraticContinuation(basket_state, coefficients)
          : fallback_continuation;

      // A_j_k is the lecture exercise event at date t_k for path j.
      const bool A_j_k = (immediate >= continuation_estimated);
      values_current[j] = A_j_k ? immediate : continuation_realized;
    }

    values_next.swap(values_current);
  }

  // X_j = exp(-r * t_first) * V_{t_first}^j.
  const double first_discount = std::exp(-risk_free_rate_ * exercise_dates_.front());
  std::vector<double> X_j(path_count, 0.0);
  for (size_t j = 0; j < path_count; ++j) {
    X_j[j] = first_discount * values_next[j];
  }

  return X_j;
}

std::pair<std::vector<double>, std::vector<double>> BermudanBasket::BuildPathValuesAndControls(
    UniformGenerator* uniform_gen,
    size_t path_count) {
  const BasketStateSimulation simulation =
      SimulateBasketStatesAndGeometricControlsByDate(uniform_gen, path_count);
  const std::vector<double> X_j =
      ComputeDiscountedPathValues(simulation.basket_states_by_date);

  return std::make_pair(X_j, simulation.geometric_controls);
}

std::vector<double> BermudanBasket::BuildAntitheticPairValues(UniformGenerator* uniform_gen,
                                                              size_t pair_count) {
  const std::vector<std::vector<double>> basket_states_by_date =
      SimulateAntitheticBasketStatesByDate(uniform_gen, pair_count);
  const std::vector<double> X_j = ComputeDiscountedPathValues(basket_states_by_date);

  std::vector<double> chi_j(pair_count, 0.0);
  for (size_t j = 0; j < pair_count; ++j) {
    chi_j[j] = 0.5 * (X_j[j] + X_j[pair_count + j]);
  }

  return chi_j;
}

std::pair<std::vector<double>, std::vector<double>>
BermudanBasket::BuildAntitheticPairValuesAndControls(UniformGenerator* uniform_gen,
                                                     size_t pair_count) {
  const BasketStateSimulation simulation =
      SimulateAntitheticBasketStatesAndGeometricControlsByDate(uniform_gen, pair_count);
  const std::vector<double> X_j =
      ComputeDiscountedPathValues(simulation.basket_states_by_date);

  std::vector<double> chi_j(pair_count, 0.0);
  std::vector<double> Y_chi_j(pair_count, 0.0);
  for (size_t j = 0; j < pair_count; ++j) {
    chi_j[j] = 0.5 * (X_j[j] + X_j[pair_count + j]);
    Y_chi_j[j] = 0.5 * (simulation.geometric_controls[j] +
                        simulation.geometric_controls[pair_count + j]);
  }

  return std::make_pair(chi_j, Y_chi_j);
}

MonteCarloSummary BermudanBasket::SummarizeSamples(const std::vector<double>& samples) {
  size_t cursor = 0;
  const std::function<double()> sampler = [&samples, &cursor]() {
    return samples[cursor++];
  };

  return MonteCarloCore::RunFixedN(sampler, samples.size(), 0.95);
}

MonteCarloSummary BermudanBasket::PriceFixedN(UniformGenerator* uniform_gen,
                                              size_t path_count) {
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen, "BermudanBasket::PriceFixedN");
  PricingHelper::ValidateCountAtLeast(path_count, 2, "BermudanBasket::PriceFixedN",
                                      "path_count");

  const std::vector<std::vector<double>> basket_states_by_date =
      SimulateBasketStatesByDate(uniform_gen, path_count);
  const std::vector<double> X_j = ComputeDiscountedPathValues(basket_states_by_date);

  return SummarizeSamples(X_j);
}

MonteCarloSummary BermudanBasket::PriceFixedNControlVariate(UniformGenerator* uniform_gen,
                                                            size_t path_count,
                                                            size_t pilot_count) {
  PricingHelper::ValidateUniformGeneratorPointer(
      uniform_gen, "BermudanBasket::PriceFixedNControlVariate");
  PricingHelper::ValidateCountAtLeast(path_count, 2,
                                      "BermudanBasket::PriceFixedNControlVariate",
                                      "path_count");
  PricingHelper::ValidateCountAtLeast(pilot_count, 2,
                                      "BermudanBasket::PriceFixedNControlVariate",
                                      "pilot_count");

  const double control_mean = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
      spot_prices_, volatilities_, weights_, strike_, maturity_, risk_free_rate_,
      dividend_yields_, correlation_matrix_);

  const std::pair<std::vector<double>, std::vector<double>> pilot_samples =
      BuildPathValuesAndControls(uniform_gen, pilot_count);
  const double beta = PricingHelper::EstimateControlVariateBeta(pilot_samples.first,
                                                                pilot_samples.second);

  const std::pair<std::vector<double>, std::vector<double>> main_samples =
      BuildPathValuesAndControls(uniform_gen, path_count);

  std::vector<double> X_j_cv(path_count, 0.0);
  for (size_t j = 0; j < path_count; ++j) {
    X_j_cv[j] = PricingHelper::ApplyControlVariate(main_samples.first[j],
                                                   main_samples.second[j],
                                                   control_mean,
                                                   beta);
  }

  return SummarizeSamples(X_j_cv);
}

MonteCarloSummary BermudanBasket::PriceFixedNAntithetic(UniformGenerator* uniform_gen,
                                                        size_t pair_count) {
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen,
                                                 "BermudanBasket::PriceFixedNAntithetic");
  PricingHelper::ValidateCountAtLeast(pair_count, 2,
                                      "BermudanBasket::PriceFixedNAntithetic", "pair_count");

  const std::vector<double> chi_j = BuildAntitheticPairValues(uniform_gen, pair_count);
  return SummarizeSamples(chi_j);
}

MonteCarloSummary BermudanBasket::PriceFixedNCumulative(UniformGenerator* uniform_gen,
                                                        size_t pair_count,
                                                        size_t pilot_count) {
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen,
                                                 "BermudanBasket::PriceFixedNCumulative");
  PricingHelper::ValidateCountAtLeast(pair_count, 2,
                                      "BermudanBasket::PriceFixedNCumulative", "pair_count");
  PricingHelper::ValidateCountAtLeast(pilot_count, 2,
                                      "BermudanBasket::PriceFixedNCumulative", "pilot_count");

  const double control_mean = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
      spot_prices_, volatilities_, weights_, strike_, maturity_, risk_free_rate_,
      dividend_yields_, correlation_matrix_);

  const std::pair<std::vector<double>, std::vector<double>> pilot_samples =
      BuildAntitheticPairValuesAndControls(uniform_gen, pilot_count);
  const double beta = PricingHelper::EstimateControlVariateBeta(pilot_samples.first,
                                                                pilot_samples.second);

  const std::pair<std::vector<double>, std::vector<double>> main_samples =
      BuildAntitheticPairValuesAndControls(uniform_gen, pair_count);

  std::vector<double> chi_j_cv(pair_count, 0.0);
  for (size_t j = 0; j < pair_count; ++j) {
    chi_j_cv[j] = PricingHelper::ApplyControlVariate(main_samples.first[j],
                                                     main_samples.second[j],
                                                     control_mean,
                                                     beta);
  }

  return SummarizeSamples(chi_j_cv);
}
