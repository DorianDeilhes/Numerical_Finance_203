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
  product.exercise_style = BermudanStyle;      // EuropeanStyle or BermudanStyle.
  product.payoff_type = BasketCallPayoff;      // Current project payoff.

  product.spot_prices = {100.0, 105.0};
  product.volatilities = {0.20, 0.25};
  product.weights = {0.60, 0.40};
  product.strike = 100.0;
  product.maturity = 1.0;
  product.risk_free_rate = 0.03;

  product.correlation_matrix = {
      {1.0, 0.35},
      {0.35, 1.0}
  };

  product.exercise_dates = {0.25, 0.50, 0.75, 1.0};
  // ======================================================================

  return product;
}

BasketPricerConfig BuildPricerConfigToModify() {
  BasketPricerConfig config;

  // ========================== EDIT PRICER HERE ==========================
  config.pricing_method = CumulativeVarianceReduction;
  config.random_generator = QuasiRandom;

  config.nb_steps = 40;
  config.path_count = 1000;
  config.pair_count = 600;
  config.pilot_count = 250;

  config.pseudo_seed_1 = 12345;
  config.pseudo_seed_2 = 67890;

  config.halton_dimension = 0;
  config.use_halton_shift = true;
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
