#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/HaltonQuasiRandom.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

enum ExerciseStyle {
  EuropeanStyle,
  BermudanStyle
};

struct BasketOptionProduct {
  ExerciseStyle exercise_style;
  std::vector<double> spot_prices;
  std::vector<double> volatilities;
  std::vector<double> weights;
  double strike;
  double maturity;
  double risk_free_rate;
  std::vector<std::vector<double>> correlation_matrix;
  std::vector<double> exercise_dates;
};

struct BasketPricerConfig {
  size_t nb_steps;
  size_t path_count;
  size_t pair_count;
  size_t pilot_count;
  size_t halton_dimension;
};

BasketOptionProduct BuildBaseProduct(ExerciseStyle exercise_style) {
  BasketOptionProduct product;
  product.exercise_style = exercise_style;
  product.spot_prices = {100.0, 105.0};
  product.volatilities = {0.20, 0.25};
  product.weights = {0.60, 0.40};
  product.strike = 100.0;
  product.maturity = 1.0;
  product.risk_free_rate = 0.03;
  product.correlation_matrix = {{1.0, 0.35}, {0.35, 1.0}};

  if (exercise_style == BermudanStyle) {
    product.exercise_dates = {0.0, 0.25, 0.50, 0.75, 1.0};
  }

  return product;
}

BasketPricerConfig BuildBasePricerConfig() {
  BasketPricerConfig config;
  config.nb_steps = 4;
  config.path_count = 3000;
  config.pair_count = 1500;
  config.pilot_count = 700;
  // For this two-asset, 4-step example, one path consumes about 8 uniforms.
  config.halton_dimension = 8;
  return config;
}

void PrintProduct(const BasketOptionProduct& product) {
  std::cout << "Product style: "
            << (product.exercise_style == EuropeanStyle ? "European" : "Bermudan")
            << "\n";
  std::cout << "Assets: " << product.spot_prices.size()
            << ", strike: " << product.strike
            << ", maturity: " << product.maturity
            << ", rate: " << product.risk_free_rate << "\n";
  if (product.exercise_style == BermudanStyle) {
    std::cout << "Exercise dates:";
    for (size_t i = 0; i < product.exercise_dates.size(); ++i) {
      std::cout << " " << product.exercise_dates[i];
    }
    std::cout << "\n";
  }
}

void PrintSummary(const std::string& label, const MonteCarloSummary& summary) {
  if (!std::isfinite(summary.mean) || !std::isfinite(summary.sampleVariance) ||
      !std::isfinite(summary.standardError)) {
    throw std::runtime_error(label + " produced non-finite Monte Carlo statistics");
  }

  std::cout << std::left << std::setw(28) << label
            << "N=" << std::setw(5) << summary.sampleSize
            << " price=" << std::setw(12) << summary.mean
            << " var=" << std::setw(12) << summary.sampleVariance
            << " stderr=" << std::setw(12) << summary.standardError
            << " CI=[" << summary.confidenceInterval.lower
            << ", " << summary.confidenceInterval.upper << "]\n";
}

void RunEuropeanWorkflow(const BasketOptionProduct& product,
                         const BasketPricerConfig& config) {
  EuropeanBasket pricer(product.spot_prices,
                        product.volatilities,
                        product.weights,
                        product.strike,
                        product.maturity,
                        product.risk_free_rate,
                        product.correlation_matrix,
                        config.nb_steps);

  std::cout << "\nEuropean pricing workflow\n";

  EcuyerCombined pseudo_base(12345, 67890);
  PrintSummary("Pseudo-random", pricer.PriceFixedN(&pseudo_base, config.path_count));

  HaltonQuasiRandom quasi_base(config.halton_dimension, true, 0.314159);
  PrintSummary("Quasi-random", pricer.PriceFixedN(&quasi_base, config.path_count));

  EcuyerCombined pseudo_control(22345, 77890);
  PrintSummary("Control variate",
               pricer.PriceFixedNControlVariate(&pseudo_control,
                                                 config.path_count,
                                                 config.pilot_count));

  EcuyerCombined pseudo_antithetic(32345, 87890);
  PrintSummary("Antithetic",
               pricer.PriceFixedNAntithetic(&pseudo_antithetic, config.pair_count));

  HaltonQuasiRandom quasi_cumulative(config.halton_dimension, true, 0.271828);
  PrintSummary("Cumulative",
               pricer.PriceFixedNCumulative(&quasi_cumulative,
                                            config.pair_count,
                                            config.pilot_count));
}

void RunBermudanWorkflow(const BasketOptionProduct& product,
                         const BasketPricerConfig& config) {
  BermudanBasket pricer(product.spot_prices,
                        product.volatilities,
                        product.weights,
                        product.strike,
                        product.maturity,
                        product.risk_free_rate,
                        product.correlation_matrix,
                        product.exercise_dates,
                        config.nb_steps);

  std::cout << "\nBermudan pricing workflow\n";

  EcuyerCombined pseudo_base(12345, 67890);
  PrintSummary("Pseudo-random", pricer.PriceFixedN(&pseudo_base, config.path_count));

  HaltonQuasiRandom quasi_base(config.halton_dimension, true, 0.314159);
  PrintSummary("Quasi-random", pricer.PriceFixedN(&quasi_base, config.path_count));

  EcuyerCombined pseudo_control(22345, 77890);
  PrintSummary("Control variate",
               pricer.PriceFixedNControlVariate(&pseudo_control,
                                                 config.path_count,
                                                 config.pilot_count));

  EcuyerCombined pseudo_antithetic(32345, 87890);
  PrintSummary("Antithetic",
               pricer.PriceFixedNAntithetic(&pseudo_antithetic, config.pair_count));

  HaltonQuasiRandom quasi_cumulative(config.halton_dimension, true, 0.271828);
  PrintSummary("Cumulative",
               pricer.PriceFixedNCumulative(&quasi_cumulative,
                                            config.pair_count,
                                            config.pilot_count));
}

}  // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  PROFESSOR BASIC TEST - BASKET OPTION PRICING\n";
    std::cout << "============================================================\n\n";

    const BasketPricerConfig config = BuildBasePricerConfig();

    const BasketOptionProduct european_product = BuildBaseProduct(EuropeanStyle);
    PrintProduct(european_product);
    RunEuropeanWorkflow(european_product, config);

    std::cout << "\n------------------------------------------------------------\n\n";

    const BasketOptionProduct bermudan_product = BuildBaseProduct(BermudanStyle);
    PrintProduct(bermudan_product);
    RunBermudanWorkflow(bermudan_product, config);

    std::cout << "\n[PROFESSOR BASIC TEST] Completed successfully.\n";
  } catch (const std::exception& exception) {
    std::cerr << "[PROFESSOR BASIC TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
