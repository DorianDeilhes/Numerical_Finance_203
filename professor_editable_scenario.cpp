#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/HaltonQuasiRandom.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

enum ExerciseStyle {
  EuropeanStyle,
  BermudanStyle
};

enum PayoffType {
  BasketCallPayoff
};

enum RandomGeneratorChoice {
  PseudoRandom,
  QuasiRandom
};

enum PricingMethod {
  BasicMonteCarlo,
  StaticControlVariate,
  AntitheticVariables,
  CumulativeVarianceReduction
};

struct BasketOptionProduct {
  ExerciseStyle exercise_style;
  PayoffType payoff_type;

  std::vector<double> spot_prices;
  std::vector<double> volatilities;
  std::vector<double> weights;
  double strike;
  double maturity;
  double risk_free_rate;
  std::vector<std::vector<double>> correlation_matrix;

  // Used only when exercise_style == BermudanStyle.
  std::vector<double> exercise_dates;
};

struct BasketPricerConfig {
  PricingMethod pricing_method;
  RandomGeneratorChoice random_generator;

  size_t nb_steps;
  size_t path_count;
  size_t pair_count;
  size_t pilot_count;

  int pseudo_seed_1;
  int pseudo_seed_2;

  // Set to 0 for automatic dimension based on basket dimension and nb_steps.
  size_t halton_dimension;
  bool use_halton_shift;
  double halton_shift_seed;
};

BasketOptionProduct BuildProductToModify() {
  BasketOptionProduct product;

  // ========================== EDIT PRODUCT HERE ==========================
  // Exercise style choices:
  // - EuropeanStyle: payoff is paid only at maturity T.
  // - BermudanStyle: payoff can be exercised on the dates in exercise_dates.
  product.exercise_style = BermudanStyle;

  // Payoff choices:
  // - BasketCallPayoff: max(sum_i weights_i * S_i(t) - strike, 0).
  // This payoff works with any number of underlyings.
  // No other payoff type is implemented in the current project.
  product.payoff_type = BasketCallPayoff;

  // Initial asset prices S_i(0). The number of values is the basket dimension.
  // Example with 2 underlyings: {100.0, 105.0}.
  // Example with 3 underlyings: {100.0, 95.0, 110.0}.
  product.spot_prices = {100.0, 105.0};

  // Black-Scholes volatilities sigma_i, one per underlying.
  // Must have the same size as spot_prices. Values are annualized decimals:
  // 0.20 means 20% volatility.
  product.volatilities = {0.20, 0.25};

  // Basket weights alpha_i, one per underlying.
  // The project statement allows possibly negative weights.
  // The weights do not have to sum to 1 in this implementation.
  product.weights = {0.60, 0.40};

  // Strike K of the basket call.
  product.strike = 100.0;

  // Maturity T in years. Example: 1.0 means one year.
  product.maturity = 1.0;

  // Risk-free rate r used in the Black-Scholes drift and discount factor.
  // Example: 0.03 means 3% annual rate.
  product.risk_free_rate = 0.03;

  // Correlation matrix R between asset Brownian motions.
  // Size must be dimension x dimension, where dimension = spot_prices.size().
  // Diagonal entries must be 1.0, the matrix must be symmetric, and it must be
  // positive semidefinite. The code internally builds B such that R = B B^T.
  // For 2 underlyings:
  //   {{1.0, rho}, {rho, 1.0}}
  // For 3 underlyings, provide a 3 x 3 matrix.
  product.correlation_matrix = {
      {1.0, 0.35},
      {0.35, 1.0}
  };

  // Bermudan exercise dates in years. Used only when exercise_style is BermudanStyle.
  // The dates must be increasing, inside [0, maturity], end exactly at maturity,
  // and align with the simulation grid defined by nb_steps below.
  // With maturity = 1.0 and nb_steps = 4, dates like
  // 0.0, 0.25, 0.50, 0.75, 1.0 are aligned because the time step is
  // 1.0 / 4 = 0.25. The first date may be 0.0, matching t0 = 0.
  // For EuropeanStyle, this vector is ignored by the pricing dispatch.
  product.exercise_dates = {0.0, 0.25, 0.50, 0.75, 1.0};
  // ======================================================================

  return product;
}

BasketPricerConfig BuildPricerConfigToModify() {
  BasketPricerConfig config;

  // ========================== EDIT PRICER HERE ==========================
  // Pricing method choices:
  // - BasicMonteCarlo: basic Monte Carlo with the selected random generator.
  // - StaticControlVariate: uses the lecture geometric basket call control
  //   Y_j = exp(-rT) * (prod_i S_i(T)^alpha_i - K)^+.
  //   This method requires weights alpha_i >= 0 and sum_i alpha_i = 1.
  // - AntitheticVariables: pairs each path with opposite Gaussian shocks.
  // - CumulativeVarianceReduction: control variate + antithetic variables
  //   with the selected random generator. Use QuasiRandom here to match the
  //   project requirement "quasi + control variate + antithetic".
  //   This also requires weights alpha_i >= 0 and sum_i alpha_i = 1.
  config.pricing_method = CumulativeVarianceReduction;

  // Random generator choices:
  // - PseudoRandom: EcuyerCombined pseudo-random uniform generator.
  // - QuasiRandom: Halton low-discrepancy generator.
  // Quasi-random numbers are deterministic and often reduce variance in practice.
  config.random_generator = QuasiRandom;

  // Number of Black-Scholes time steps between 0 and maturity.
  // Larger values can increase accuracy for path-dependent/exercise-date products,
  // but they also increase runtime. Bermudan exercise_dates must align with this grid.
  config.nb_steps = 4;

  // Number of Monte Carlo paths for BasicMonteCarlo and StaticControlVariate.
  // Must be at least 2. Larger values reduce the standard error roughly as 1/sqrt(N).
  config.path_count = 3000;

  // Number of direct/antithetic pairs for AntitheticVariables and
  // CumulativeVarianceReduction. One pair means two simulated paths, but one
  // estimator sample chi_j = 0.5 * (X_direct + X_antithetic).
  config.pair_count = 1500;

  // Number of pilot samples used to estimate the control variate coefficient beta.
  // StaticControlVariate uses pilot_count paths.
  // CumulativeVarianceReduction uses pilot_count antithetic pairs.
  config.pilot_count = 700;

  // Seeds for the EcuyerCombined pseudo-random generator.
  // Used only when random_generator = PseudoRandom.
  config.pseudo_seed_1 = 12345;
  config.pseudo_seed_2 = 67890;

  // Halton dimension for QuasiRandom.
  // Set to 0 for automatic choice based on basket dimension and nb_steps.
  // If you set it manually, it must be between 1 and 4096.
  config.halton_dimension = 0;

  // Optional deterministic shift for the Halton sequence.
  // Keeping it true is a simple way to avoid always starting from exactly the
  // same unshifted low-discrepancy points.
  config.use_halton_shift = true;

  // Seed used to build the deterministic Halton shift.
  // Used only when random_generator = QuasiRandom and use_halton_shift = true.
  config.halton_shift_seed = 0.314159;
  // ======================================================================

  return config;
}

const char* ExerciseStyleName(ExerciseStyle exercise_style) {
  return exercise_style == EuropeanStyle ? "European" : "Bermudan";
}

const char* PricingMethodName(PricingMethod pricing_method) {
  if (pricing_method == BasicMonteCarlo) {
    return "Basic Monte Carlo";
  }
  if (pricing_method == StaticControlVariate) {
    return "Static Control Variate";
  }
  if (pricing_method == AntitheticVariables) {
    return "Antithetic Variables";
  }
  return "Cumulative Variance Reduction";
}

const char* RandomGeneratorName(RandomGeneratorChoice random_generator) {
  return random_generator == PseudoRandom ? "Pseudo-random" : "Quasi-random Halton";
}

size_t AutomaticHaltonDimension(const BasketOptionProduct& product,
                                const BasketPricerConfig& config) {
  const size_t normal_count = product.spot_prices.size() * config.nb_steps;
  const size_t box_muller_uniform_count = (normal_count % 2 == 0)
      ? normal_count
      : normal_count + 1;
  if (box_muller_uniform_count == 0) {
    return 1;
  }
  if (box_muller_uniform_count > 4096) {
    return 4096;
  }
  return box_muller_uniform_count;
}

std::unique_ptr<UniformGenerator> BuildGenerator(const BasketOptionProduct& product,
                                                 const BasketPricerConfig& config) {
  if (config.random_generator == PseudoRandom) {
    return std::unique_ptr<UniformGenerator>(
        new EcuyerCombined(config.pseudo_seed_1, config.pseudo_seed_2));
  }

  const size_t dimension = (config.halton_dimension == 0)
      ? AutomaticHaltonDimension(product, config)
      : config.halton_dimension;
  return std::unique_ptr<UniformGenerator>(
      new HaltonQuasiRandom(dimension, config.use_halton_shift, config.halton_shift_seed));
}

void PrintProduct(const BasketOptionProduct& product) {
  std::cout << "Product\n";
  std::cout << "  Style: " << ExerciseStyleName(product.exercise_style) << "\n";
  std::cout << "  Payoff: Basket Call\n";
  std::cout << "  Assets: " << product.spot_prices.size() << "\n";
  std::cout << "  Strike: " << product.strike
            << ", maturity: " << product.maturity
            << ", rate: " << product.risk_free_rate << "\n";

  if (product.exercise_style == BermudanStyle) {
    std::cout << "  Exercise dates:";
    for (size_t i = 0; i < product.exercise_dates.size(); ++i) {
      std::cout << " " << product.exercise_dates[i];
    }
    std::cout << "\n";
  }
}

void PrintConfig(const BasketPricerConfig& config) {
  std::cout << "Pricer config\n";
  std::cout << "  Method: " << PricingMethodName(config.pricing_method) << "\n";
  std::cout << "  Generator: " << RandomGeneratorName(config.random_generator) << "\n";
  std::cout << "  nb_steps: " << config.nb_steps
            << ", path_count: " << config.path_count
            << ", pair_count: " << config.pair_count
            << ", pilot_count: " << config.pilot_count << "\n";
}

void PrintSummary(const MonteCarloSummary& summary) {
  if (!std::isfinite(summary.mean) || !std::isfinite(summary.sampleVariance) ||
      !std::isfinite(summary.standardError)) {
    throw std::runtime_error("Scenario produced non-finite Monte Carlo statistics");
  }

  std::cout << std::setprecision(10);
  std::cout << "\nResult\n";
  std::cout << "  sample size       : " << summary.sampleSize << "\n";
  std::cout << "  price             : " << summary.mean << "\n";
  std::cout << "  sample variance   : " << summary.sampleVariance << "\n";
  std::cout << "  standard error    : " << summary.standardError << "\n";
  std::cout << "  confidence level  : " << summary.confidenceInterval.confidenceLevel << "\n";
  std::cout << "  confidence interval: ["
            << summary.confidenceInterval.lower << ", "
            << summary.confidenceInterval.upper << "]\n";
}

class BasketScenarioPricer {
 public:
  MonteCarloSummary Price(const BasketOptionProduct& product,
                          const BasketPricerConfig& config) const {
    if (product.payoff_type != BasketCallPayoff) {
      throw std::runtime_error("Only BasketCallPayoff is implemented in this project");
    }

    if (product.exercise_style == EuropeanStyle) {
      return PriceEuropean(product, config);
    }
    return PriceBermudan(product, config);
  }

 private:
  MonteCarloSummary PriceEuropean(const BasketOptionProduct& product,
                                  const BasketPricerConfig& config) const {
    EuropeanBasket pricer(product.spot_prices,
                          product.volatilities,
                          product.weights,
                          product.strike,
                          product.maturity,
                          product.risk_free_rate,
                          product.correlation_matrix,
                          config.nb_steps);

    std::unique_ptr<UniformGenerator> generator = BuildGenerator(product, config);
    if (config.pricing_method == BasicMonteCarlo) {
      return pricer.PriceFixedN(generator.get(), config.path_count);
    }
    if (config.pricing_method == StaticControlVariate) {
      return pricer.PriceFixedNControlVariate(generator.get(),
                                             config.path_count,
                                             config.pilot_count);
    }
    if (config.pricing_method == AntitheticVariables) {
      return pricer.PriceFixedNAntithetic(generator.get(), config.pair_count);
    }
    return pricer.PriceFixedNCumulative(generator.get(),
                                        config.pair_count,
                                        config.pilot_count);
  }

  MonteCarloSummary PriceBermudan(const BasketOptionProduct& product,
                                  const BasketPricerConfig& config) const {
    BermudanBasket pricer(product.spot_prices,
                          product.volatilities,
                          product.weights,
                          product.strike,
                          product.maturity,
                          product.risk_free_rate,
                          product.correlation_matrix,
                          product.exercise_dates,
                          config.nb_steps);

    std::unique_ptr<UniformGenerator> generator = BuildGenerator(product, config);
    if (config.pricing_method == BasicMonteCarlo) {
      return pricer.PriceFixedN(generator.get(), config.path_count);
    }
    if (config.pricing_method == StaticControlVariate) {
      return pricer.PriceFixedNControlVariate(generator.get(),
                                             config.path_count,
                                             config.pilot_count);
    }
    if (config.pricing_method == AntitheticVariables) {
      return pricer.PriceFixedNAntithetic(generator.get(), config.pair_count);
    }
    return pricer.PriceFixedNCumulative(generator.get(),
                                        config.pair_count,
                                        config.pilot_count);
  }
};

}  // namespace

int main() {
  try {
    const BasketOptionProduct product = BuildProductToModify();
    const BasketPricerConfig config = BuildPricerConfigToModify();
    const BasketScenarioPricer pricer;

    std::cout << "============================================================\n";
    std::cout << "  PROFESSOR EDITABLE SCENARIO\n";
    std::cout << "============================================================\n\n";

    PrintProduct(product);
    std::cout << "\n";
    PrintConfig(config);

    const MonteCarloSummary summary = pricer.Price(product, config);
    PrintSummary(summary);

    std::cout << "\n[PROFESSOR EDITABLE SCENARIO] Completed successfully.\n";
  } catch (const std::exception& exception) {
    std::cerr << "[PROFESSOR EDITABLE SCENARIO] Failure: "
              << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
