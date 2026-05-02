#include "Pricing/EuropeanBasket.h"

#include "MonteCarlo/Helper/BasketCallPayoff.h"
#include "MonteCarlo/Helper/DiscountPayoff.h"
#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalND.h"
#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalNDAntithetic.h"
#include "Pricing/Helper/ApplyControlVariate.h"
#include "Pricing/Helper/BuildLoadingMatrixFromCorrelation.h"
#include "Pricing/Helper/DiscountedGeometricBasketCall.h"
#include "Pricing/Helper/EstimateControlVariateBeta.h"
#include "Pricing/Helper/KnownMeanDiscountedGeometricBasketCall.h"
#include "Pricing/Helper/ValidateEuropeanBasketInputs.h"
#include "Pricing/Helper/ValidatePricingMethodInputs.h"
#include <cmath>
#include <stdexcept>
#include <utility>

EuropeanBasket::EuropeanBasket(const std::vector<double>& spot_prices,
                               const std::vector<double>& volatilities,
                               const std::vector<double>& weights,
                               double strike,
                               double maturity,
                               double risk_free_rate,
                               const std::vector<std::vector<double>>& correlation_matrix,
                               size_t nb_steps)
    : dimension_(spot_prices.size()),
      spot_prices_(spot_prices),
      volatilities_(volatilities),
      weights_(weights),
      strike_(strike),
      maturity_(maturity),
      risk_free_rate_(risk_free_rate),
      correlation_matrix_(correlation_matrix),
      loading_matrix_(),
      nb_steps_(nb_steps) {
  ValidateInputs();
  loading_matrix_ = PricingHelper::BuildLoadingMatrixFromCorrelation(correlation_matrix_);
}

void EuropeanBasket::ValidateInputs() {
  PricingHelper::ValidateEuropeanBasketInputs(
      spot_prices_, volatilities_, weights_, strike_, maturity_, risk_free_rate_,
      correlation_matrix_, nb_steps_);
}

double EuropeanBasket::SinglePathPayoff(UniformGenerator* uniform_gen) {
  std::vector<double> terminalSpots = MonteCarloHelper::SimulateGeometricBrownianTerminalND(
      uniform_gen, spot_prices_, volatilities_, risk_free_rate_, &loading_matrix_, 0.0,
      maturity_, nb_steps_);

  const double payoff = MonteCarloHelper::BasketCallPayoff(terminalSpots, weights_, strike_);
  return MonteCarloHelper::DiscountPayoff(payoff, risk_free_rate_, maturity_);
}

std::pair<double, double> EuropeanBasket::SinglePathPayoffAndControl(
    UniformGenerator* uniform_gen) {
  std::vector<double> terminalSpots = MonteCarloHelper::SimulateGeometricBrownianTerminalND(
      uniform_gen, spot_prices_, volatilities_, risk_free_rate_, &loading_matrix_, 0.0,
      maturity_, nb_steps_);

  const double callPayoff = MonteCarloHelper::BasketCallPayoff(terminalSpots, weights_, strike_);
  const double targetSample =
      MonteCarloHelper::DiscountPayoff(callPayoff, risk_free_rate_, maturity_);

  const double controlSample = PricingHelper::DiscountedGeometricBasketCall(
      terminalSpots, weights_, strike_, risk_free_rate_, maturity_);

  return std::make_pair(targetSample, controlSample);
}

std::pair<double, double> EuropeanBasket::SinglePathAntitheticAverages(
    UniformGenerator* uniform_gen) {
  MonteCarloHelper::TerminalSpotsAntitheticPair terminals =
      MonteCarloHelper::SimulateGeometricBrownianTerminalNDAntithetic(
          uniform_gen, spot_prices_, volatilities_, risk_free_rate_, &loading_matrix_,
          0.0, maturity_, nb_steps_);

  const double directCall =
      MonteCarloHelper::BasketCallPayoff(terminals.direct, weights_, strike_);
  const double antiCall =
      MonteCarloHelper::BasketCallPayoff(terminals.antithetic, weights_, strike_);

  const double targetDirect =
      MonteCarloHelper::DiscountPayoff(directCall, risk_free_rate_, maturity_);
  const double targetAnti =
      MonteCarloHelper::DiscountPayoff(antiCall, risk_free_rate_, maturity_);
  const double targetAverage = 0.5 * (targetDirect + targetAnti);

  const double controlDirect = PricingHelper::DiscountedGeometricBasketCall(
      terminals.direct, weights_, strike_, risk_free_rate_, maturity_);
  const double controlAnti = PricingHelper::DiscountedGeometricBasketCall(
      terminals.antithetic, weights_, strike_, risk_free_rate_, maturity_);
  const double controlAverage = 0.5 * (controlDirect + controlAnti);

  return std::make_pair(targetAverage, controlAverage);
}

double EuropeanBasket::SinglePathAntitheticPayoffOnly(UniformGenerator* uniform_gen) {
  MonteCarloHelper::TerminalSpotsAntitheticPair terminals =
      MonteCarloHelper::SimulateGeometricBrownianTerminalNDAntithetic(
          uniform_gen, spot_prices_, volatilities_, risk_free_rate_, &loading_matrix_,
          0.0, maturity_, nb_steps_);

  const double directCall =
      MonteCarloHelper::BasketCallPayoff(terminals.direct, weights_, strike_);
  const double antiCall =
      MonteCarloHelper::BasketCallPayoff(terminals.antithetic, weights_, strike_);

  const double targetDirect =
      MonteCarloHelper::DiscountPayoff(directCall, risk_free_rate_, maturity_);
  const double targetAnti =
      MonteCarloHelper::DiscountPayoff(antiCall, risk_free_rate_, maturity_);

  return 0.5 * (targetDirect + targetAnti);
}

MonteCarloSummary EuropeanBasket::PriceFixedN(UniformGenerator* uniform_gen,
                                               size_t num_samples) {
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen, "EuropeanBasket::PriceFixedN");
  PricingHelper::ValidateCountAtLeast(num_samples, 2, "EuropeanBasket::PriceFixedN",
                                      "num_samples");

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
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen,
                                                 "EuropeanBasket::PriceFixedPrecision");

  // Sampler: single path payoff.
  const std::function<double()> sampler = [this, uniform_gen]() {
    return SinglePathPayoff(uniform_gen);
  };

  // Run Monte Carlo with adaptive sample size to meet precision target.
  return MonteCarloCore::RunFixedPrecision(sampler, epsilon, 0.95, min_samples, max_samples);
}

MonteCarloSummary EuropeanBasket::PriceFixedNAntithetic(UniformGenerator* uniform_gen,
                                                         size_t pair_count) {
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen,
                                                 "EuropeanBasket::PriceFixedNAntithetic");
  PricingHelper::ValidateCountAtLeast(pair_count, 2,
                                      "EuropeanBasket::PriceFixedNAntithetic", "pair_count");

  const std::function<double()> sampler = [this, uniform_gen]() {
    return SinglePathAntitheticPayoffOnly(uniform_gen);
  };

  return MonteCarloCore::RunFixedN(sampler, pair_count, 0.95);
}

MonteCarloSummary EuropeanBasket::PriceFixedNControlVariate(UniformGenerator* uniform_gen,
                                                             size_t sample_count,
                                                             size_t pilot_count) {
  PricingHelper::ValidateUniformGeneratorPointer(
      uniform_gen, "EuropeanBasket::PriceFixedNControlVariate");
  PricingHelper::ValidateCountAtLeast(sample_count, 2,
                                      "EuropeanBasket::PriceFixedNControlVariate",
                                      "sample_count");
  PricingHelper::ValidateCountAtLeast(pilot_count, 2,
                                      "EuropeanBasket::PriceFixedNControlVariate",
                                      "pilot_count");

  const double controlMean = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
      spot_prices_, volatilities_, weights_, strike_, maturity_, risk_free_rate_,
      correlation_matrix_);

  std::vector<double> pilotTarget;
  std::vector<double> pilotControl;
  pilotTarget.reserve(pilot_count);
  pilotControl.reserve(pilot_count);

  for (size_t i = 0; i < pilot_count; ++i) {
    const std::pair<double, double> sample = SinglePathPayoffAndControl(uniform_gen);
    pilotTarget.push_back(sample.first);
    pilotControl.push_back(sample.second);
  }

  const double beta = PricingHelper::EstimateControlVariateBeta(pilotTarget, pilotControl);

  const std::function<double()> correctedSampler = [this, uniform_gen, beta, controlMean]() {
    const std::pair<double, double> sample = SinglePathPayoffAndControl(uniform_gen);
    return PricingHelper::ApplyControlVariate(sample.first, sample.second, controlMean, beta);
  };

  return MonteCarloCore::RunFixedN(correctedSampler, sample_count, 0.95);
}

MonteCarloSummary EuropeanBasket::PriceFixedNCumulative(UniformGenerator* uniform_gen,
                                                         size_t pair_count,
                                                         size_t pilot_count) {
  PricingHelper::ValidateUniformGeneratorPointer(uniform_gen,
                                                 "EuropeanBasket::PriceFixedNCumulative");
  PricingHelper::ValidateCountAtLeast(pair_count, 2,
                                      "EuropeanBasket::PriceFixedNCumulative", "pair_count");
  PricingHelper::ValidateCountAtLeast(pilot_count, 2,
                                      "EuropeanBasket::PriceFixedNCumulative", "pilot_count");

  const double controlMean = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
      spot_prices_, volatilities_, weights_, strike_, maturity_, risk_free_rate_,
      correlation_matrix_);

  std::vector<double> pilotTarget;
  std::vector<double> pilotControl;
  pilotTarget.reserve(pilot_count);
  pilotControl.reserve(pilot_count);

  for (size_t i = 0; i < pilot_count; ++i) {
    const std::pair<double, double> sample = SinglePathAntitheticAverages(uniform_gen);
    pilotTarget.push_back(sample.first);
    pilotControl.push_back(sample.second);
  }

  const double beta = PricingHelper::EstimateControlVariateBeta(pilotTarget, pilotControl);

  const std::function<double()> correctedSampler = [this, uniform_gen, beta, controlMean]() {
    const std::pair<double, double> sample = SinglePathAntitheticAverages(uniform_gen);
    return PricingHelper::ApplyControlVariate(sample.first, sample.second, controlMean, beta);
  };

  return MonteCarloCore::RunFixedN(correctedSampler, pair_count, 0.95);
}
