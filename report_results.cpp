#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/HaltonQuasiRandom.h"

#include <cmath>
#include <fstream>
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

enum VarianceReductionLayer {
  BasicPseudoRandom,
  QuasiRandomOnly,
  QuasiRandomControlVariate,
  QuasiRandomControlVariateAntithetic
};

struct BasketOptionProduct {
  std::string scenario_name;
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

struct ReportPricerConfig {
  size_t nb_steps;
  size_t path_count;
  size_t pair_count;
  size_t pilot_count;
  size_t halton_dimension;
  bool use_halton_shift;
  double halton_shift_seed;
  double epsilon;
};

struct ReportRow {
  std::string scenario_name;
  std::string exercise_style;
  std::string method_name;
  std::string random_generator_name;
  size_t estimator_sample_count;
  size_t main_path_count;
  size_t pilot_path_count;
  size_t total_path_count;
  double price;
  double sample_variance;
  double standard_error;
  double confidence_level;
  double confidence_lower;
  double confidence_upper;
  double confidence_half_width;
  double n_star_epsilon_alpha;
  double n_star_path_equivalent;
  double variance_reduction_factor;
  double required_path_gain;
};

const char* ExerciseStyleName(ExerciseStyle exercise_style) {
  return exercise_style == EuropeanStyle ? "European" : "Bermudan";
}

const char* MethodName(VarianceReductionLayer method) {
  if (method == BasicPseudoRandom) {
    return "Basic pseudo-random";
  }
  if (method == QuasiRandomOnly) {
    return "Quasi-random";
  }
  if (method == QuasiRandomControlVariate) {
    return "Quasi-random + control variate";
  }
  return "Quasi-random + control variate + antithetic";
}

const char* GeneratorName(VarianceReductionLayer method) {
  return method == BasicPseudoRandom ? "EcuyerCombined" : "HaltonQuasiRandom";
}

size_t AutomaticHaltonDimension(const BasketOptionProduct& product,
                                const ReportPricerConfig& config) {
  const size_t normal_count = product.spot_prices.size() * config.nb_steps;
  const size_t box_muller_uniform_count =
      (normal_count % 2 == 0) ? normal_count : normal_count + 1;

  if (box_muller_uniform_count == 0) {
    return 1;
  }
  if (box_muller_uniform_count > 4096) {
    return 4096;
  }
  return box_muller_uniform_count;
}

std::unique_ptr<UniformGenerator> BuildGenerator(const BasketOptionProduct& product,
                                                 const ReportPricerConfig& config,
                                                 VarianceReductionLayer method,
                                                 int seed_shift) {
  if (method == BasicPseudoRandom) {
    return std::unique_ptr<UniformGenerator>(
        new EcuyerCombined(12345 + seed_shift, 67890 + seed_shift));
  }

  const size_t dimension = (config.halton_dimension == 0)
      ? AutomaticHaltonDimension(product, config)
      : config.halton_dimension;
  const double shifted_seed =
      config.halton_shift_seed + 0.037 * static_cast<double>(seed_shift);

  return std::unique_ptr<UniformGenerator>(
      new HaltonQuasiRandom(dimension, config.use_halton_shift, shifted_seed));
}

MonteCarloSummary PriceEuropean(const BasketOptionProduct& product,
                                const ReportPricerConfig& config,
                                VarianceReductionLayer method,
                                int seed_shift) {
  EuropeanBasket pricer(product.spot_prices,
                        product.volatilities,
                        product.weights,
                        product.strike,
                        product.maturity,
                        product.risk_free_rate,
                        product.correlation_matrix,
                        config.nb_steps);

  std::unique_ptr<UniformGenerator> generator =
      BuildGenerator(product, config, method, seed_shift);

  if (method == BasicPseudoRandom || method == QuasiRandomOnly) {
    return pricer.PriceFixedN(generator.get(), config.path_count);
  }
  if (method == QuasiRandomControlVariate) {
    return pricer.PriceFixedNControlVariate(generator.get(),
                                           config.path_count,
                                           config.pilot_count);
  }
  return pricer.PriceFixedNCumulative(generator.get(),
                                      config.pair_count,
                                      config.pilot_count);
}

MonteCarloSummary PriceBermudan(const BasketOptionProduct& product,
                                const ReportPricerConfig& config,
                                VarianceReductionLayer method,
                                int seed_shift) {
  BermudanBasket pricer(product.spot_prices,
                        product.volatilities,
                        product.weights,
                        product.strike,
                        product.maturity,
                        product.risk_free_rate,
                        product.correlation_matrix,
                        product.exercise_dates,
                        config.nb_steps);

  std::unique_ptr<UniformGenerator> generator =
      BuildGenerator(product, config, method, seed_shift);

  if (method == BasicPseudoRandom || method == QuasiRandomOnly) {
    return pricer.PriceFixedN(generator.get(), config.path_count);
  }
  if (method == QuasiRandomControlVariate) {
    return pricer.PriceFixedNControlVariate(generator.get(),
                                           config.path_count,
                                           config.pilot_count);
  }
  return pricer.PriceFixedNCumulative(generator.get(),
                                      config.pair_count,
                                      config.pilot_count);
}

MonteCarloSummary PriceProduct(const BasketOptionProduct& product,
                               const ReportPricerConfig& config,
                               VarianceReductionLayer method,
                               int seed_shift) {
  if (product.exercise_style == EuropeanStyle) {
    return PriceEuropean(product, config, method, seed_shift);
  }
  return PriceBermudan(product, config, method, seed_shift);
}

size_t MainPathCount(const ReportPricerConfig& config,
                     VarianceReductionLayer method) {
  if (method == QuasiRandomControlVariateAntithetic) {
    return 2 * config.pair_count;
  }
  return config.path_count;
}

size_t PilotPathCount(const ReportPricerConfig& config,
                      VarianceReductionLayer method) {
  if (method == QuasiRandomControlVariate) {
    return config.pilot_count;
  }
  if (method == QuasiRandomControlVariateAntithetic) {
    return 2 * config.pilot_count;
  }
  return 0;
}

double PathMultiplier(VarianceReductionLayer method) {
  return method == QuasiRandomControlVariateAntithetic ? 2.0 : 1.0;
}

ReportRow BuildReportRow(const BasketOptionProduct& product,
                         const ReportPricerConfig& config,
                         VarianceReductionLayer method,
                         const MonteCarloSummary& summary,
                         double baseline_n_star_path_equivalent) {
  if (!std::isfinite(summary.mean) || !std::isfinite(summary.sampleVariance) ||
      !std::isfinite(summary.standardError)) {
    throw std::runtime_error("Report run produced non-finite Monte Carlo statistics");
  }

  const double z_score = summary.confidenceInterval.zScore;
  const double n_star =
      z_score * z_score * summary.sampleVariance / (config.epsilon * config.epsilon);
  const double n_star_path_equivalent = n_star * PathMultiplier(method);

  ReportRow row;
  row.scenario_name = product.scenario_name;
  row.exercise_style = ExerciseStyleName(product.exercise_style);
  row.method_name = MethodName(method);
  row.random_generator_name = GeneratorName(method);
  row.estimator_sample_count = summary.sampleSize;
  row.main_path_count = MainPathCount(config, method);
  row.pilot_path_count = PilotPathCount(config, method);
  row.total_path_count = row.main_path_count + row.pilot_path_count;
  row.price = summary.mean;
  row.sample_variance = summary.sampleVariance;
  row.standard_error = summary.standardError;
  row.confidence_level = summary.confidenceInterval.confidenceLevel;
  row.confidence_lower = summary.confidenceInterval.lower;
  row.confidence_upper = summary.confidenceInterval.upper;
  row.confidence_half_width = summary.confidenceInterval.halfWidth;
  row.n_star_epsilon_alpha = n_star;
  row.n_star_path_equivalent = n_star_path_equivalent;
  row.variance_reduction_factor = 1.0;
  row.required_path_gain = 1.0;

  if (method != BasicPseudoRandom && n_star_path_equivalent > 0.0) {
    row.required_path_gain = baseline_n_star_path_equivalent / n_star_path_equivalent;
  }

  return row;
}

void WriteCsvHeader(std::ostream& out) {
  out << "scenario,exercise_style,method,random_generator,"
      << "estimator_sample_count,main_path_count,pilot_path_count,total_path_count,"
      << "price,sample_variance,standard_error,confidence_level,"
      << "confidence_lower,confidence_upper,confidence_half_width,"
      << "N_star_epsilon_alpha,N_star_path_equivalent,"
      << "variance_reduction_factor,required_path_gain\n";
}

void WriteCsvRow(std::ostream& out, const ReportRow& row) {
  out << row.scenario_name << ","
      << row.exercise_style << ","
      << row.method_name << ","
      << row.random_generator_name << ","
      << row.estimator_sample_count << ","
      << row.main_path_count << ","
      << row.pilot_path_count << ","
      << row.total_path_count << ","
      << row.price << ","
      << row.sample_variance << ","
      << row.standard_error << ","
      << row.confidence_level << ","
      << row.confidence_lower << ","
      << row.confidence_upper << ","
      << row.confidence_half_width << ","
      << row.n_star_epsilon_alpha << ","
      << row.n_star_path_equivalent << ","
      << row.variance_reduction_factor << ","
      << row.required_path_gain << "\n";
}

void PrintConsoleRow(const ReportRow& row) {
  std::cout << std::left << std::setw(22) << row.scenario_name
            << std::setw(10) << row.exercise_style
            << std::setw(50) << row.method_name
            << "price=" << std::setw(12) << row.price
            << "var=" << std::setw(12) << row.sample_variance
            << "VRF=" << std::setw(9) << row.variance_reduction_factor
            << "N_gain=" << row.required_path_gain << "\n";
}

BasketOptionProduct BuildBalancedTwoAssetScenario(ExerciseStyle exercise_style) {
  BasketOptionProduct product;
  product.scenario_name = "balanced_2_assets";
  product.exercise_style = exercise_style;
  product.spot_prices = {100.0, 105.0};
  product.volatilities = {0.20, 0.25};
  product.weights = {0.60, 0.40};
  product.strike = 100.0;
  product.maturity = 1.0;
  product.risk_free_rate = 0.03;
  product.correlation_matrix = {{1.0, 0.35}, {0.35, 1.0}};
  product.exercise_dates = {0.25, 0.50, 0.75, 1.0};
  return product;
}

BasketOptionProduct BuildMixedWeightThreeAssetScenario(ExerciseStyle exercise_style) {
  BasketOptionProduct product;
  product.scenario_name = "mixed_weights_3_assets";
  product.exercise_style = exercise_style;
  product.spot_prices = {100.0, 95.0, 110.0};
  product.volatilities = {0.18, 0.22, 0.30};
  product.weights = {0.70, 0.50, -0.20};
  product.strike = 102.0;
  product.maturity = 1.0;
  product.risk_free_rate = 0.03;
  product.correlation_matrix = {
      {1.0, 0.20, -0.10},
      {0.20, 1.0, 0.25},
      {-0.10, 0.25, 1.0}
  };
  product.exercise_dates = {0.25, 0.50, 0.75, 1.0};
  return product;
}

ReportPricerConfig BuildReportConfig() {
  ReportPricerConfig config;
  config.nb_steps = 4;
  config.path_count = 3000;
  config.pair_count = 1500;
  config.pilot_count = 700;
  config.halton_dimension = 0;
  config.use_halton_shift = true;
  config.halton_shift_seed = 0.314159;
  config.epsilon = 0.25;
  return config;
}

std::vector<BasketOptionProduct> BuildReportProducts() {
  std::vector<BasketOptionProduct> products;
  products.push_back(BuildBalancedTwoAssetScenario(EuropeanStyle));
  products.push_back(BuildBalancedTwoAssetScenario(BermudanStyle));
  products.push_back(BuildMixedWeightThreeAssetScenario(EuropeanStyle));
  products.push_back(BuildMixedWeightThreeAssetScenario(BermudanStyle));
  return products;
}

std::vector<VarianceReductionLayer> BuildReportMethods() {
  std::vector<VarianceReductionLayer> methods;
  methods.push_back(BasicPseudoRandom);
  methods.push_back(QuasiRandomOnly);
  methods.push_back(QuasiRandomControlVariate);
  methods.push_back(QuasiRandomControlVariateAntithetic);
  return methods;
}

std::vector<ReportRow> RunReport(const ReportPricerConfig& config) {
  const std::vector<BasketOptionProduct> products = BuildReportProducts();
  const std::vector<VarianceReductionLayer> methods = BuildReportMethods();
  std::vector<ReportRow> rows;

  for (size_t product_index = 0; product_index < products.size(); ++product_index) {
    const BasketOptionProduct& product = products[product_index];
    double baseline_variance = 0.0;
    double baseline_n_star_path_equivalent = 0.0;

    for (size_t method_index = 0; method_index < methods.size(); ++method_index) {
      const VarianceReductionLayer method = methods[method_index];
      const int seed_shift = static_cast<int>(10 * product_index + method_index);
      const MonteCarloSummary summary = PriceProduct(product, config, method, seed_shift);

      ReportRow row =
          BuildReportRow(product, config, method, summary, baseline_n_star_path_equivalent);

      if (method == BasicPseudoRandom) {
        baseline_variance = summary.sampleVariance;
        baseline_n_star_path_equivalent = row.n_star_path_equivalent;
      }

      if (method != BasicPseudoRandom && row.sample_variance > 0.0) {
        row.variance_reduction_factor = baseline_variance / row.sample_variance;
      }

      rows.push_back(row);
      PrintConsoleRow(row);
    }
  }

  return rows;
}

void WriteCsvFile(const std::string& output_file, const std::vector<ReportRow>& rows) {
  std::ofstream out(output_file.c_str());
  if (!out) {
    throw std::runtime_error("Could not open report CSV output file");
  }

  out << std::setprecision(12);
  WriteCsvHeader(out);
  for (size_t i = 0; i < rows.size(); ++i) {
    WriteCsvRow(out, rows[i]);
  }
}

}  // namespace

int main() {
  try {
    const ReportPricerConfig config = BuildReportConfig();

    std::cout << "============================================================\n";
    std::cout << "  REPORT RESULTS - VARIANCE REDUCTION TABLES\n";
    std::cout << "============================================================\n\n";
    std::cout << "epsilon for N_star(epsilon, alpha): " << config.epsilon << "\n";
    std::cout << "path_count=" << config.path_count
              << ", pair_count=" << config.pair_count
              << ", pilot_count=" << config.pilot_count
              << ", nb_steps=" << config.nb_steps << "\n\n";

    const std::vector<ReportRow> rows = RunReport(config);
    const std::string output_file = "report_results.csv";
    WriteCsvFile(output_file, rows);

    std::cout << "\nCSV written to " << output_file << "\n";
    std::cout << "[REPORT RESULTS] Completed successfully.\n";
  } catch (const std::exception& exception) {
    std::cerr << "[REPORT RESULTS] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
