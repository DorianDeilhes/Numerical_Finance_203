#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalND.h"
#include "Pricing/Helper/BuildLoadingMatrixFromCorrelation.h"
#include "Pricing/Helper/DiscountedGeometricBasketCall.h"
#include "Pricing/Helper/KnownMeanDiscountedGeometricBasketCall.h"
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

// ============================================================
// QUICK GUIDE — what to tweak before running
// ============================================================
// 1. Edit BuildProductToModify()   — basket parameters, style, dates.
// 2. Edit BuildPricerConfigToModify() — method, generator, path count.
// 3. Set RUN_COMPARISON = true     — side-by-side VR comparison table.
// 4. Set RUN_VALIDATION_SUITE = true — 5 built-in theory checks.
// 5. Set RUN_ROBUSTNESS_SUITE = true — edge-case checks from Claude.md.
//
// Rules of thumb:
//   - path_count: 1000 (fast, rough), 5000 (balanced), 20000 (precise).
//   - Control variate methods require weights >= 0 summing to 1.
//   - Bermudan exercise_dates must be multiples of maturity/nb_steps.
//   - pilot_count must be < path_count (or < pair_count for cumulative).
// ============================================================

// Toggle to compare all 4 variance-reduction layers on your scenario.
static const bool RUN_COMPARISON = false;

// Toggle to run 5 built-in theory checks after your scenario.
static const bool RUN_VALIDATION_SUITE = true;

// Toggle to run robustness / edge-case checks after the validation suite.
static const bool RUN_ROBUSTNESS_SUITE = true;

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

  // Continuous dividend yield q_i per asset (annualized). 0.0 = no dividend.
  // Must have one entry per underlying (same size as spot_prices).
  std::vector<double> dividend_yields;

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

  // Continuous dividend yield q_i per asset (annualized decimal), one per underlying.
  // Under Black-Scholes with dividends, the drift of asset i becomes (r - q_i).
  // Set all entries to 0.0 for non-dividend-paying assets.
  // Example with 2 underlyings: {0.02, 0.05} means 2% and 5% annual dividend yields.
  product.dividend_yields = {0.0, 0.0};

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
  if (!product.dividend_yields.empty()) {
    std::cout << "  Dividend yields:";
    for (size_t i = 0; i < product.dividend_yields.size(); ++i) {
      std::cout << " " << product.dividend_yields[i];
    }
    std::cout << "\n";
  }
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

void Check(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename Callable>
void ExpectThrows(const std::string& message, Callable&& callable) {
  bool threw = false;
  try {
    callable();
  } catch (const std::exception&) {
    threw = true;
  }
  Check(threw, message);
}

void CheckFiniteSummary(const MonteCarloSummary& summary, const std::string& label) {
  Check(std::isfinite(summary.mean), label + " mean must be finite");
  Check(std::isfinite(summary.sampleVariance), label + " variance must be finite");
  Check(std::isfinite(summary.standardError), label + " standard error must be finite");
  Check(summary.sampleSize >= 2, label + " sample size must be >= 2");
}

struct SampleStats {
  double mean;
  double sample_variance;
  double standard_error;
};

SampleStats ComputeSampleStats(const std::vector<double>& samples) {
  Check(samples.size() >= 2, "ComputeSampleStats requires at least 2 samples");

  double sum = 0.0;
  for (size_t i = 0; i < samples.size(); ++i) {
    sum += samples[i];
  }
  const double mean = sum / static_cast<double>(samples.size());

  double squared_sum = 0.0;
  for (size_t i = 0; i < samples.size(); ++i) {
    const double centered = samples[i] - mean;
    squared_sum += centered * centered;
  }

  const double sample_variance =
      squared_sum / static_cast<double>(samples.size() - 1);
  const double standard_error =
      std::sqrt(sample_variance / static_cast<double>(samples.size()));

  SampleStats stats;
  stats.mean = mean;
  stats.sample_variance = sample_variance;
  stats.standard_error = standard_error;
  return stats;
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
                          config.nb_steps,
                          product.dividend_yields);

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
                          config.nb_steps,
                          product.dividend_yields);

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

// ============================================================
// BLACK-SCHOLES CLOSED FORM  (single-asset reference)
// ============================================================

double NormalCDF(double x) {
  return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

// Returns the Black-Scholes (Merton 1973) price for a European call.
// q = continuous dividend yield (annualized). q=0 gives the standard BS formula.
double BlackScholesCall(double S, double K, double T, double r, double sigma,
                        double q = 0.0) {
  if (sigma < 1e-12) {
    return std::max(S * std::exp(-q * T) - K * std::exp(-r * T), 0.0);
  }
  const double d1 = (std::log(S / K) + (r - q + 0.5 * sigma * sigma) * T)
                    / (sigma * std::sqrt(T));
  const double d2 = d1 - sigma * std::sqrt(T);
  return S * std::exp(-q * T) * NormalCDF(d1) - K * std::exp(-r * T) * NormalCDF(d2);
}

// ============================================================
// ALL-METHODS COMPARISON TABLE
// Runs the 4 cumulative VR layers on whatever product/config
// the user has configured and prints a side-by-side table.
// Enable via:  static const bool RUN_COMPARISON = true;
// ============================================================

void RunComparisonTable(const BasketOptionProduct& product,
                        const BasketPricerConfig& base_config,
                        const BasketScenarioPricer& pricer) {
  struct MethodRow {
    const char* label;
    PricingMethod method;
    RandomGeneratorChoice rng;
  };
  const MethodRow rows[] = {
    {"1. Pseudo-random (baseline)",  BasicMonteCarlo,             PseudoRandom},
    {"2. Quasi-random",              BasicMonteCarlo,             QuasiRandom},
    {"3. Quasi + control variate",   StaticControlVariate,        QuasiRandom},
    {"4. Quasi + CV + antithetic",   CumulativeVarianceReduction, QuasiRandom},
  };

  std::cout << "\n============================================================\n"
            << "  ALL-METHODS COMPARISON TABLE\n"
            << "============================================================\n";
  std::cout << std::left
            << std::setw(32) << "Method"
            << std::setw(12) << "Price"
            << std::setw(14) << "Variance"
            << std::setw(12) << "StdErr"
            << "VRF\n"
            << std::string(72, '-') << "\n";

  double baseline_var = -1.0;
  for (const auto& row : rows) {
    BasketPricerConfig cfg  = base_config;
    cfg.pricing_method      = row.method;
    cfg.random_generator    = row.rng;
    try {
      const MonteCarloSummary s = pricer.Price(product, cfg);
      if (baseline_var < 0.0) baseline_var = s.sampleVariance;
      const double vrf = (baseline_var > 0.0 && s.sampleVariance > 0.0)
                         ? baseline_var / s.sampleVariance : 1.0;
      std::cout << std::setprecision(6) << std::left
                << std::setw(32) << row.label
                << std::setw(12) << s.mean
                << std::setw(14) << s.sampleVariance
                << std::setw(12) << s.standardError
                << vrf << "\n";
    } catch (const std::exception& e) {
      std::cout << std::left << std::setw(32) << row.label
                << "  [skipped: " << e.what() << "]\n";
    }
  }
  std::cout << "  VRF = var_baseline / var_method  (>1 means improvement)\n"
            << "  Rows 3 & 4 require basket weights >= 0 and summing to 1.\n";
}

// ============================================================
// VALIDATION SUITE — 5 theory checks
// Enable via:  static const bool RUN_VALIDATION_SUITE = true;
// ============================================================

void RunValidationSuite() {
  int passed = 0;
  const int total = 5;

  auto tag = [](bool ok) -> const char* { return ok ? "  PASS" : "  FAIL ***"; };

  std::cout << "\n============================================================\n"
            << "  VALIDATION SUITE  (" << total << " theory checks)\n"
            << "============================================================\n";

  // ----------------------------------------------------------
  // Test 1: Zero volatility — price must equal max(S - K*exp(-rT), 0)
  // With sigma=0 the GBM is deterministic: every path is identical.
  // Variance should collapse to ~0 and the MC price must match exactly.
  // ----------------------------------------------------------
  std::cout << "\n[1/" << total << "] Zero-volatility pricing\n";
  {
    const double S = 100.0, K = 97.0, T = 1.0, r = 0.03;
    const double expected = std::max(S - K * std::exp(-r * T), 0.0);
    EuropeanBasket p({S}, {0.0}, {1.0}, K, T, r, {{1.0}}, 50);
    EcuyerCombined g(1, 2);
    const MonteCarloSummary s = p.PriceFixedN(&g, 500);
    const bool ok = std::abs(s.mean - expected) < 0.05;
    std::cout << "  Expected (analytical): " << expected << "\n"
              << "  MC price:              " << s.mean
              << "  (SE=" << s.standardError << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 2: 1-asset basket must match Black-Scholes closed form.
  // A basket with one asset and weight=1 is a plain BS call.
  // Pass if |MC - BS| < 3 standard errors.
  // ----------------------------------------------------------
  std::cout << "\n[2/" << total << "] 1-asset basket vs Black-Scholes formula\n";
  {
    const double S = 100.0, K = 100.0, T = 1.0, r = 0.05, sigma = 0.20;
    const double bs = BlackScholesCall(S, K, T, r, sigma);
    EuropeanBasket p({S}, {sigma}, {1.0}, K, T, r, {{1.0}}, 100);
    EcuyerCombined g(1111, 2222);
    const MonteCarloSummary s = p.PriceFixedN(&g, 5000);
    const double tol = 3.0 * s.standardError;
    const bool ok = std::abs(s.mean - bs) < tol;
    std::cout << "  Black-Scholes reference: " << bs << "\n"
              << "  MC price:                " << s.mean
              << "  (SE=" << s.standardError << ", 3*SE=" << tol << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 3: Bermudan price >= European price.
  // Early exercise adds optionality, so Bermudan can never be cheaper.
  // Pass if Bermudan mean >= European mean - 3 * combined SE.
  // ----------------------------------------------------------
  std::cout << "\n[3/" << total << "] Bermudan price >= European price\n";
  {
    const std::vector<double> sp = {100.0, 105.0};
    const std::vector<double> v  = {0.20, 0.25};
    const std::vector<double> w  = {0.60, 0.40};
    const double K = 100.0, T = 1.0, r = 0.03;
    const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
    EuropeanBasket euro(sp, v, w, K, T, r, corr, 4);
    BermudanBasket berm(sp, v, w, K, T, r, corr, {0.0, 0.25, 0.50, 0.75, 1.0}, 4);
    EcuyerCombined ge(11, 22), gb(33, 44);
    const MonteCarloSummary se = euro.PriceFixedN(&ge, 4000);
    const MonteCarloSummary sb = berm.PriceFixedN(&gb, 4000);
    const double cse = std::sqrt(se.sampleVariance / se.sampleSize +
                                 sb.sampleVariance / sb.sampleSize);
    const bool ok = sb.mean >= se.mean - 3.0 * cse;
    std::cout << "  European price:  " << se.mean << "  (SE=" << se.standardError << ")\n"
              << "  Bermudan price:  " << sb.mean << "  (SE=" << sb.standardError << ")\n"
              << "  Difference Berm-Euro: " << sb.mean - se.mean
              << "  (threshold > " << -3.0 * cse << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 4: Bermudan with only one exercise date at T == European.
  // No early exercise is possible, so both must agree within MC noise.
  // Same seeds → should produce identical paths.
  // ----------------------------------------------------------
  std::cout << "\n[4/" << total << "] Bermudan(T only) == European (same seed)\n";
  {
    const std::vector<double> sp = {100.0, 105.0};
    const std::vector<double> v  = {0.20, 0.25};
    const std::vector<double> w  = {0.60, 0.40};
    const double K = 100.0, T = 1.0, r = 0.03;
    const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
    EuropeanBasket euro(sp, v, w, K, T, r, corr, 4);
    BermudanBasket berm(sp, v, w, K, T, r, corr, {T}, 4);
    EcuyerCombined ge(55, 66), gb(55, 66);
    const MonteCarloSummary se = euro.PriceFixedN(&ge, 3000);
    const MonteCarloSummary sb = berm.PriceFixedN(&gb, 3000);
    const double cse = std::sqrt(se.sampleVariance / se.sampleSize +
                                 sb.sampleVariance / sb.sampleSize);
    const double diff = std::abs(sb.mean - se.mean);
    const bool ok = diff < 3.0 * cse;
    std::cout << "  European price:         " << se.mean
              << "  (SE=" << se.standardError << ")\n"
              << "  Bermudan(T only) price: " << sb.mean
              << "  (SE=" << sb.standardError << ")\n"
              << "  |Difference|: " << diff << "  (< 3*SE_comb=" << 3.0 * cse << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 5: Variance reduction ordering (European, 2 assets).
  // Stacking VR techniques must never increase variance:
  //   Cumulative var <= Antithetic var <= Basic var.
  // ----------------------------------------------------------
  std::cout << "\n[5/" << total << "] Variance reduction ordering (European)\n";
  {
    const std::vector<double> sp = {100.0, 105.0};
    const std::vector<double> v  = {0.20, 0.25};
    const std::vector<double> w  = {0.60, 0.40};
    const double K = 100.0, T = 1.0, r = 0.03;
    const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
    EuropeanBasket p(sp, v, w, K, T, r, corr, 4);
    EcuyerCombined gb(11, 22), ga(33, 44), gc(55, 66);
    const auto sB = p.PriceFixedN(&gb, 3000);
    const auto sA = p.PriceFixedNAntithetic(&ga, 1500);
    const auto sC = p.PriceFixedNCumulative(&gc, 1500, 500);
    const bool antith_ok = sA.sampleVariance < sB.sampleVariance;
    const bool cumul_ok  = sC.sampleVariance < sA.sampleVariance;
    const bool ok = antith_ok && cumul_ok;
    std::cout << "  Basic      var=" << sB.sampleVariance << "  VRF=1.00\n"
              << "  Antithetic var=" << sA.sampleVariance
              << "  VRF=" << sB.sampleVariance / sA.sampleVariance
              << (antith_ok ? "  (< basic: OK)" : "  (>= basic: BAD)") << "\n"
              << "  Cumulative var=" << sC.sampleVariance
              << "  VRF=" << sB.sampleVariance / sC.sampleVariance
              << (cumul_ok ? "  (< antithetic: OK)" : "  (>= antithetic: BAD)") << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Bonus — Dividend yield creates an early-exercise premium on a call.
  //
  // Theory: early exercise of a call is optimal when q > r roughly, because
  // you capture the dividend income by holding the stock rather than the option.
  // At q=0: Bermudan ≈ European (no incentive to exercise early).
  // At q > r: Bermudan > European with a measurable premium.
  // This is the simplest way to demonstrate early-exercise value without r<0.
  // ----------------------------------------------------------
  std::cout << "\n[Bonus A] Dividend yield — early-exercise premium on a call\n";
  {
    const double S = 100.0, K = 100.0, T = 1.0, r = 0.03, sigma = 0.20;
    const std::vector<std::vector<double>> corr1 = {{1.0}};
    const std::vector<double> dates = {0.0, 0.25, 0.50, 0.75, 1.0};

    std::cout << "  " << std::left
              << std::setw(8)  << "q"
              << std::setw(14) << "BS-European"
              << std::setw(14) << "MC-European"
              << std::setw(14) << "MC-Bermudan"
              << "Premium (Berm-Euro)\n"
              << "  " << std::string(64, '-') << "\n";

    for (double q : {0.00, 0.03, 0.06, 0.10}) {
      const double bs = BlackScholesCall(S, K, T, r, sigma, q);
      EuropeanBasket euro({S}, {sigma}, {1.0}, K, T, r, corr1, 4, {q});
      BermudanBasket berm({S}, {sigma}, {1.0}, K, T, r, corr1, dates, 4, {q});
      EcuyerCombined ge(111, 222), gb(333, 444);
      const auto se = euro.PriceFixedN(&ge, 8000);
      const auto sb = berm.PriceFixedN(&gb, 8000);
      const double premium = sb.mean - se.mean;
      const double cse = std::sqrt(se.sampleVariance / se.sampleSize +
                                   sb.sampleVariance / sb.sampleSize);
      std::cout << "  " << std::setprecision(4) << std::left
                << std::setw(8)  << q
                << std::setw(14) << bs
                << std::setw(14) << se.mean
                << std::setw(14) << sb.mean
                << premium << "  (±" << cse << ")"
                << (premium > 2.0 * cse ? "  *" : "") << "\n";
    }
    std::cout << "  * = premium exceeds 2 combined SE (statistically visible)\n";
  }

  // ----------------------------------------------------------
  // Bonus — Rate sensitivity: Bermudan vs European at r=0 vs r=0.03
  //
  // Theory: for a call on non-dividend-paying assets —
  //   r > 0  →  C_bermudan ≈ C_european  (early exercise suboptimal)
  //   r = 0  →  C_bermudan ≈ C_european  (lower bound touches, premium ≈ 0)
  //   r < 0  →  C_bermudan > C_european  (BLOCKED by validator: r must be >= 0)
  //
  // With r<0 you lose money by leaving K uninvested, so exercising early
  // (paying K now rather than later) becomes attractive — same mechanism as
  // cash dividends on the underlying.  The current implementation enforces
  // r >= 0, so r<0 cannot be tested directly.
  // ----------------------------------------------------------
  std::cout << "\n[Bonus] Bermudan vs European early-exercise premium vs rate\n";
  {
    const std::vector<double> sp = {100.0, 105.0};
    const std::vector<double> v  = {0.20, 0.25};
    const std::vector<double> w  = {0.60, 0.40};
    const double K = 100.0, T = 1.0;
    const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
    const std::vector<double> dates = {0.0, 0.25, 0.50, 0.75, 1.0};

    std::cout << "  " << std::left << std::setw(8) << "rate"
              << std::setw(16) << "European"
              << std::setw(16) << "Bermudan"
              << "Premium (Berm-Euro)\n"
              << "  " << std::string(56, '-') << "\n";

    for (double r : {0.05, 0.03, 0.01, 0.00}) {
      EuropeanBasket euro(sp, v, w, K, T, r, corr, 4);
      BermudanBasket berm(sp, v, w, K, T, r, corr, dates, 4);
      EcuyerCombined ge(11, 22), gb(33, 44);
      const auto se = euro.PriceFixedN(&ge, 5000);
      const auto sb = berm.PriceFixedN(&gb, 5000);
      const double cse = std::sqrt(se.sampleVariance / se.sampleSize +
                                   sb.sampleVariance / sb.sampleSize);
      std::cout << "  " << std::setw(8) << r
                << std::setw(16) << se.mean
                << std::setw(16) << sb.mean
                << sb.mean - se.mean
                << "  (±" << cse << ")\n";
    }
    std::cout << "  Note: r < 0 throws std::invalid_argument (validator enforces r >= 0).\n"
              << "  Premium stays near zero for all r >= 0 on a call without dividends.\n";
  }

  std::cout << "\n============================================================\n"
            << "  VALIDATION: " << passed << "/" << total << " tests passed\n"
            << "============================================================\n";
}

void RunRobustnessSuite() {
  int passed = 0;
  const int total = 21;

  auto tag = [](bool ok) -> const char* { return ok ? "  PASS" : "  FAIL ***"; };

  std::cout << "\n============================================================\n"
            << "  ROBUSTNESS / EDGE-CASE SUITE  (" << total << " checks)\n"
            << "============================================================\n";

  // ----------------------------------------------------------
  // Test 1: Identity correlation should price cleanly.
  // ----------------------------------------------------------
  std::cout << "\n[1/" << total << "] Identity correlation matrix\n";
  {
    EuropeanBasket euro({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 0.0}, {0.0, 1.0}}, 4);
    EcuyerCombined g(101, 202);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 4000);
    CheckFiniteSummary(s, "Identity correlation");
    const bool ok = std::isfinite(s.mean) && s.mean > 0.0;
    std::cout << "  Price: " << s.mean << "  (SE=" << s.standardError << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 2: Perfect correlation rho=1 should be accepted via
  // singular-matrix fallback logic and still produce a price.
  // ----------------------------------------------------------
  std::cout << "\n[2/" << total << "] Perfect correlation rho = 1\n";
  {
    EuropeanBasket euro({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 1.0}, {1.0, 1.0}}, 4);
    EcuyerCombined g(303, 404);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 4000);
    CheckFiniteSummary(s, "Perfect correlation");
    const bool ok = std::isfinite(s.mean) && s.mean > 0.0;
    std::cout << "  Price: " << s.mean << "  (SE=" << s.standardError << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 3: Zero weights should behave gracefully.
  // Here the second asset has weight 0, so the price should match
  // the corresponding one-asset Black-Scholes call.
  // ----------------------------------------------------------
  std::cout << "\n[3/" << total << "] Zero-weight asset\n";
  {
    const double S = 100.0, K = 100.0, T = 1.0, r = 0.05, sigma = 0.20;
    const double bs = BlackScholesCall(S, K, T, r, sigma);
    EuropeanBasket euro({S, 150.0}, {sigma, 0.50}, {1.0, 0.0},
                        K, T, r, {{1.0, 0.0}, {0.0, 1.0}}, 100);
    EcuyerCombined g(505, 606);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 6000);
    const double tol = 3.0 * s.standardError;
    const bool ok = std::abs(s.mean - bs) < tol;
    std::cout << "  Reference BS: " << bs << "\n"
              << "  MC price:     " << s.mean
              << "  (SE=" << s.standardError << ", 3*SE=" << tol << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 4: Large sample count should remain stable.
  // ----------------------------------------------------------
  std::cout << "\n[4/" << total << "] Large-N Monte Carlo run\n";
  {
    const size_t sample_count = 50000;
    EuropeanBasket euro({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}}, 4);
    EcuyerCombined g(707, 808);
    const MonteCarloSummary s = euro.PriceFixedN(&g, sample_count);
    const bool ok = std::isfinite(s.mean) && s.sampleSize == sample_count;
    std::cout << "  Price: " << s.mean << "  sample size: " << s.sampleSize
              << "  SE=" << s.standardError << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 5: Negative weights are allowed for European pricing.
  // ----------------------------------------------------------
  std::cout << "\n[5/" << total << "] Negative basket weights (European)\n";
  {
    EuropeanBasket euro({100.0, 90.0}, {0.20, 0.20}, {1.20, -0.20},
                        100.0, 1.0, 0.03, {{1.0, 0.30}, {0.30, 1.0}}, 4);
    EcuyerCombined g(909, 1001);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 5000);
    CheckFiniteSummary(s, "Negative weights");
    const bool ok = std::isfinite(s.mean) && s.mean >= 0.0;
    std::cout << "  Price: " << s.mean << "  (SE=" << s.standardError << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 6: K=0 gives payoff S(T) for a 1-asset call, so the
  // discounted expectation equals S0 when q=0.
  // ----------------------------------------------------------
  std::cout << "\n[6/" << total << "] Zero strike K = 0\n";
  {
    const double S = 100.0, T = 1.0, r = 0.05, sigma = 0.20;
    const double expected = S;
    EuropeanBasket euro({S}, {sigma}, {1.0}, 0.0, T, r, {{1.0}}, 100);
    EcuyerCombined g(1102, 1203);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 5000);
    const double tol = 3.0 * s.standardError;
    const bool ok = std::abs(s.mean - expected) < tol;
    std::cout << "  Expected: " << expected << "\n"
              << "  MC price: " << s.mean
              << "  (SE=" << s.standardError << ", 3*SE=" << tol << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 7: r=0 and sigma=0 gives a deterministic payoff.
  // ----------------------------------------------------------
  std::cout << "\n[7/" << total << "] Deterministic world: r = 0, sigma = 0\n";
  {
    const double S = 100.0, K = 97.0, T = 1.0;
    const double expected = std::max(S - K, 0.0);
    EuropeanBasket euro({S}, {0.0}, {1.0}, K, T, 0.0, {{1.0}}, 20);
    EcuyerCombined g(1304, 1405);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 200);
    const bool ok = std::abs(s.mean - expected) < 1e-12 && s.sampleVariance == 0.0;
    std::cout << "  Expected: " << expected << "\n"
              << "  MC price: " << s.mean << "  variance=" << s.sampleVariance << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 8: Exercise date at t=0 must be accepted and should
  // not reduce the Bermudan value relative to the same schedule
  // without t=0.
  // ----------------------------------------------------------
  std::cout << "\n[8/" << total << "] Exercise date at t = 0\n";
  {
    BermudanBasket with_t0({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                           {{1.0}}, {0.0, 0.5, 1.0}, 4, {0.10});
    BermudanBasket without_t0({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                              {{1.0}}, {0.5, 1.0}, 4, {0.10});
    EcuyerCombined g1(1506, 1607), g2(1708, 1809);
    const MonteCarloSummary s1 = with_t0.PriceFixedN(&g1, 6000);
    const MonteCarloSummary s2 = without_t0.PriceFixedN(&g2, 6000);
    const double cse = std::sqrt(s1.sampleVariance / s1.sampleSize +
                                 s2.sampleVariance / s2.sampleSize);
    const bool ok = s1.mean >= s2.mean - 3.0 * cse;
    std::cout << "  With t0:    " << s1.mean << "\n"
              << "  Without t0: " << s2.mean
              << "  (threshold > " << s2.mean - 3.0 * cse << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 9: Invalid inputs must throw cleanly.
  // ----------------------------------------------------------
  std::cout << "\n[9/" << total << "] Invalid input rejection\n";
  {
    bool ok = true;
    try {
      ExpectThrows("Negative dividend yield should throw", []() {
        EuropeanBasket euro({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                            {{1.0}}, 4, {-0.01});
      });
      ExpectThrows("Misaligned exercise date should throw on pricing", []() {
        BermudanBasket berm({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                            {{1.0}}, {0.3333, 1.0}, 4);
        EcuyerCombined g(1910, 2011);
        (void)berm.PriceFixedN(&g, 10);
      });
    } catch (const std::exception&) {
      ok = false;
    }
    std::cout << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 10: Antithetic pricing with dividends should remain
  // stable, and a higher dividend should reduce European value.
  // ----------------------------------------------------------
  std::cout << "\n[10/" << total << "] Dividend + antithetic path\n";
  {
    EuropeanBasket low_q({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                         {{1.0}}, 40, {0.00});
    EuropeanBasket high_q({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                          {{1.0}}, 40, {0.08});
    EcuyerCombined g1(2101, 2202), g2(2303, 2404);
    const MonteCarloSummary s_low = low_q.PriceFixedNAntithetic(&g1, 3000);
    const MonteCarloSummary s_high = high_q.PriceFixedNAntithetic(&g2, 3000);
    CheckFiniteSummary(s_low, "Dividend antithetic low-q");
    CheckFiniteSummary(s_high, "Dividend antithetic high-q");
    const double cse = std::sqrt(s_low.sampleVariance / s_low.sampleSize +
                                 s_high.sampleVariance / s_high.sampleSize);
    const bool ok = s_high.mean < s_low.mean - 2.0 * cse;
    std::cout << "  q=0.00 price: " << s_low.mean << "\n"
              << "  q=0.08 price: " << s_high.mean
              << "  (difference=" << s_high.mean - s_low.mean << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 11: Control variate pricing with dividends should
  // remain stable, and a higher dividend should reduce value.
  // ----------------------------------------------------------
  std::cout << "\n[11/" << total << "] Dividend + control variate path\n";
  {
    EuropeanBasket low_q({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                         100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}},
                         20, {0.00, 0.00});
    EuropeanBasket high_q({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                          100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}},
                          20, {0.08, 0.05});
    EcuyerCombined g1(2505, 2606), g2(2707, 2808);
    const MonteCarloSummary s_low = low_q.PriceFixedNControlVariate(&g1, 3000, 700);
    const MonteCarloSummary s_high = high_q.PriceFixedNControlVariate(&g2, 3000, 700);
    CheckFiniteSummary(s_low, "Dividend control variate low-q");
    CheckFiniteSummary(s_high, "Dividend control variate high-q");
    const double cse = std::sqrt(s_low.sampleVariance / s_low.sampleSize +
                                 s_high.sampleVariance / s_high.sampleSize);
    const bool ok = s_high.mean < s_low.mean - 2.0 * cse;
    std::cout << "  q_low price:  " << s_low.mean << "\n"
              << "  q_high price: " << s_high.mean
              << "  (difference=" << s_high.mean - s_low.mean << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 12: Cumulative pricing with dividends should show a
  // visible Bermudan premium at high dividend yield.
  // ----------------------------------------------------------
  std::cout << "\n[12/" << total << "] Dividend + cumulative path\n";
  {
    EuropeanBasket euro({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                        {{1.0}}, 20, {0.10});
    BermudanBasket berm({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                        {{1.0}}, {0.0, 0.25, 0.50, 0.75, 1.0}, 20, {0.10});
    HaltonQuasiRandom g1(20, true, 0.123456), g2(20, true, 0.654321);
    const MonteCarloSummary s_e = euro.PriceFixedNCumulative(&g1, 2500, 700);
    const MonteCarloSummary s_b = berm.PriceFixedNCumulative(&g2, 2500, 700);
    CheckFiniteSummary(s_e, "Dividend cumulative euro");
    CheckFiniteSummary(s_b, "Dividend cumulative berm");
    const double cse = std::sqrt(s_e.sampleVariance / s_e.sampleSize +
                                 s_b.sampleVariance / s_b.sampleSize);
    const bool ok = s_b.mean > s_e.mean + 2.0 * cse;
    std::cout << "  Euro: " << s_e.mean << "\n"
              << "  Berm: " << s_b.mean
              << "  premium=" << s_b.mean - s_e.mean
              << "  (2*CSE=" << 2.0 * cse << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 13: The analytic geometric-control mean should reduce
  // to Black-Scholes-Merton in the 1-asset case with dividends.
  // ----------------------------------------------------------
  std::cout << "\n[13/" << total << "] KnownMean helper in 1D with dividend\n";
  {
    const double helper = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
        {100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.05, {0.04}, {{1.0}});
    const double bs = BlackScholesCall(100.0, 100.0, 1.0, 0.05, 0.20, 0.04);
    const bool ok = std::abs(helper - bs) < 1e-12;
    std::cout << "  Helper: " << helper << "\n"
              << "  BSM:    " << bs << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 14: The analytic geometric-control mean should match
  // a direct Monte Carlo estimate in a 2-asset basket.
  // ----------------------------------------------------------
  std::cout << "\n[14/" << total << "] KnownMean helper vs MC in 2D\n";
  {
    const std::vector<double> spots = {100.0, 105.0};
    const std::vector<double> vols = {0.20, 0.25};
    const std::vector<double> weights = {0.60, 0.40};
    const double strike = 100.0;
    const double maturity = 1.0;
    const double rate = 0.03;
    const std::vector<double> dividends = {0.04, 0.02};
    const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
    const double analytic = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
        spots, vols, weights, strike, maturity, rate, dividends, corr);

    std::vector<std::vector<double>> loading =
        PricingHelper::BuildLoadingMatrixFromCorrelation(corr);
    EcuyerCombined rng(2909, 3010);
    std::vector<double> samples;
    const size_t sample_count = 8000;
    samples.reserve(sample_count);
    for (size_t i = 0; i < sample_count; ++i) {
      const std::vector<double> terminal =
          MonteCarloHelper::SimulateGeometricBrownianTerminalND(
              &rng, spots, vols, rate, dividends, &loading, 0.0, maturity, 20);
      samples.push_back(PricingHelper::DiscountedGeometricBasketCall(
          terminal, weights, strike, rate, maturity));
    }
    const SampleStats stats = ComputeSampleStats(samples);
    const bool ok = std::abs(stats.mean - analytic) < 3.0 * stats.standard_error;
    std::cout << "  Analytic: " << analytic << "\n"
              << "  MC mean:  " << stats.mean
              << "  (SE=" << stats.standard_error << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 15: Strong t0 immediate-exercise scenario.
  // Deterministic declining stock under high dividend yield:
  // with t0, the value should equal the intrinsic value exactly.
  // ----------------------------------------------------------
  std::cout << "\n[15/" << total << "] Strong immediate exercise at t0\n";
  {
    const double intrinsic = 20.0;
    BermudanBasket with_t0({120.0}, {0.0}, {1.0}, 100.0, 1.0, 0.03,
                           {{1.0}}, {0.0, 0.25, 0.50, 0.75, 1.0}, 20, {0.20});
    BermudanBasket without_t0({120.0}, {0.0}, {1.0}, 100.0, 1.0, 0.03,
                              {{1.0}}, {0.25, 0.50, 0.75, 1.0}, 20, {0.20});
    EcuyerCombined g1(3111, 3212), g2(3313, 3414);
    const MonteCarloSummary s1 = with_t0.PriceFixedN(&g1, 200);
    const MonteCarloSummary s2 = without_t0.PriceFixedN(&g2, 200);
    const bool ok = std::abs(s1.mean - intrinsic) < 1e-12 && s1.mean > s2.mean + 1.0;
    std::cout << "  With t0:    " << s1.mean << "\n"
              << "  Without t0: " << s2.mean << "\n"
              << "  Intrinsic:  " << intrinsic << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 16: Deterministic multi-asset dividend sensitivity.
  // Higher dividends reduce the European value and increase
  // the Bermudan premium in a stable 2-asset setup.
  // ----------------------------------------------------------
  std::cout << "\n[16/" << total << "] Multi-asset dividend sensitivity\n";
  {
    const std::vector<double> sp = {120.0, 110.0};
    const std::vector<double> vol = {0.0, 0.0};
    const std::vector<double> w = {0.50, 0.50};
    const std::vector<std::vector<double>> corr = {{1.0, 0.0}, {0.0, 1.0}};
    const std::vector<double> dates = {0.25, 0.50, 0.75, 1.0};

    EuropeanBasket euro_low(sp, vol, w, 100.0, 1.0, 0.03, corr, 20, {0.00, 0.00});
    EuropeanBasket euro_high(sp, vol, w, 100.0, 1.0, 0.03, corr, 20, {0.20, 0.15});
    BermudanBasket berm_low(sp, vol, w, 100.0, 1.0, 0.03, corr, dates, 20, {0.00, 0.00});
    BermudanBasket berm_high(sp, vol, w, 100.0, 1.0, 0.03, corr, dates, 20, {0.20, 0.15});
    EcuyerCombined g1(3515, 3616), g2(3717, 3818), g3(3919, 4020), g4(4121, 4222);
    const MonteCarloSummary se_low = euro_low.PriceFixedN(&g1, 200);
    const MonteCarloSummary se_high = euro_high.PriceFixedN(&g2, 200);
    const MonteCarloSummary sb_low = berm_low.PriceFixedN(&g3, 200);
    const MonteCarloSummary sb_high = berm_high.PriceFixedN(&g4, 200);
    const double premium_low = sb_low.mean - se_low.mean;
    const double premium_high = sb_high.mean - se_high.mean;
    const bool ok = se_high.mean < se_low.mean && premium_high > premium_low + 1e-12;
    std::cout << "  Euro low-q:  " << se_low.mean << "\n"
              << "  Euro high-q: " << se_high.mean << "\n"
              << "  Premium low-q:  " << premium_low << "\n"
              << "  Premium high-q: " << premium_high << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 17: All-zero weights should produce a zero price for
  // both European and Bermudan calls.
  // ----------------------------------------------------------
  std::cout << "\n[17/" << total << "] All-zero basket weights\n";
  {
    EuropeanBasket euro({100.0, 120.0}, {0.20, 0.25}, {0.0, 0.0},
                        100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}}, 4);
    BermudanBasket berm({100.0, 120.0}, {0.20, 0.25}, {0.0, 0.0},
                        100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}},
                        {0.0, 0.5, 1.0}, 4);
    EcuyerCombined g1(4323, 4424), g2(4525, 4626);
    const MonteCarloSummary se = euro.PriceFixedN(&g1, 500);
    const MonteCarloSummary sb = berm.PriceFixedN(&g2, 500);
    const bool ok = std::abs(se.mean) < 1e-12 && std::abs(sb.mean) < 1e-12 &&
                    se.sampleVariance == 0.0 && sb.sampleVariance == 0.0;
    std::cout << "  European: " << se.mean << "\n"
              << "  Bermudan: " << sb.mean << "\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 18: Deep ITM European call should match Black-Scholes.
  // ----------------------------------------------------------
  std::cout << "\n[18/" << total << "] Deep ITM European pricing\n";
  {
    const double S = 200.0, K = 50.0, T = 1.0, r = 0.03, sigma = 0.20;
    const double bs = BlackScholesCall(S, K, T, r, sigma);
    EuropeanBasket euro({S}, {sigma}, {1.0}, K, T, r, {{1.0}}, 40);
    EcuyerCombined g(4727, 4828);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 4000);
    const bool ok = std::abs(s.mean - bs) < 3.0 * s.standardError;
    std::cout << "  BS: " << bs << "\n"
              << "  MC: " << s.mean
              << "  (SE=" << s.standardError << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 19: Deep OTM European call should be small and match
  // Black-Scholes within Monte Carlo error.
  // ----------------------------------------------------------
  std::cout << "\n[19/" << total << "] Deep OTM European pricing\n";
  {
    const double S = 70.0, K = 130.0, T = 1.0, r = 0.03, sigma = 0.20;
    const double bs = BlackScholesCall(S, K, T, r, sigma);
    EuropeanBasket euro({S}, {sigma}, {1.0}, K, T, r, {{1.0}}, 40);
    EcuyerCombined g(4929, 5030);
    const MonteCarloSummary s = euro.PriceFixedN(&g, 12000);
    const bool ok = s.mean >= 0.0 && std::abs(s.mean - bs) < 3.0 * s.standardError;
    std::cout << "  BS: " << bs << "\n"
              << "  MC: " << s.mean
              << "  (SE=" << s.standardError << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 20: Deep ITM Bermudan with high dividends should show
  // a visible premium over the corresponding European call.
  // ----------------------------------------------------------
  std::cout << "\n[20/" << total << "] Deep ITM Bermudan premium\n";
  {
    EuropeanBasket euro({200.0}, {0.20}, {1.0}, 50.0, 1.0, 0.03,
                        {{1.0}}, 20, {0.12});
    BermudanBasket berm({200.0}, {0.20}, {1.0}, 50.0, 1.0, 0.03,
                        {{1.0}}, {0.25, 0.50, 0.75, 1.0}, 20, {0.12});
    EcuyerCombined g1(5131, 5232), g2(5333, 5434);
    const MonteCarloSummary se = euro.PriceFixedN(&g1, 5000);
    const MonteCarloSummary sb = berm.PriceFixedN(&g2, 5000);
    const double cse = std::sqrt(se.sampleVariance / se.sampleSize +
                                 sb.sampleVariance / sb.sampleSize);
    const bool ok = sb.mean > se.mean + 2.0 * cse;
    std::cout << "  Euro: " << se.mean << "\n"
              << "  Berm: " << sb.mean
              << "  premium=" << sb.mean - se.mean
              << "  (2*CSE=" << 2.0 * cse << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  // ----------------------------------------------------------
  // Test 21: Deep OTM Bermudan and European prices should both
  // stay near zero, with no visible premium.
  // ----------------------------------------------------------
  std::cout << "\n[21/" << total << "] Deep OTM Bermudan vs European\n";
  {
    EuropeanBasket euro({60.0}, {0.20}, {1.0}, 140.0, 1.0, 0.03,
                        {{1.0}}, 20, {0.05});
    BermudanBasket berm({60.0}, {0.20}, {1.0}, 140.0, 1.0, 0.03,
                        {{1.0}}, {0.25, 0.50, 0.75, 1.0}, 20, {0.05});
    EcuyerCombined g1(5535, 5636), g2(5737, 5838);
    const MonteCarloSummary se = euro.PriceFixedN(&g1, 12000);
    const MonteCarloSummary sb = berm.PriceFixedN(&g2, 12000);
    const double cse = std::sqrt(se.sampleVariance / se.sampleSize +
                                 sb.sampleVariance / sb.sampleSize);
    const bool ok = se.mean >= 0.0 && sb.mean >= 0.0 &&
                    std::abs(sb.mean - se.mean) <= 3.0 * cse;
    std::cout << "  Euro: " << se.mean << "\n"
              << "  Berm: " << sb.mean
              << "  |diff|=" << std::abs(sb.mean - se.mean)
              << "  (3*CSE=" << 3.0 * cse << ")\n"
              << tag(ok) << "\n";
    if (ok) ++passed;
  }

  std::cout << "\n============================================================\n"
            << "  ROBUSTNESS: " << passed << "/" << total << " checks passed\n"
            << "============================================================\n";
}

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

    if (RUN_COMPARISON) {
      RunComparisonTable(product, config, pricer);
    }

    if (RUN_VALIDATION_SUITE) {
      RunValidationSuite();
    }

    if (RUN_ROBUSTNESS_SUITE) {
      RunRobustnessSuite();
    }

    std::cout << "\n[PROFESSOR EDITABLE SCENARIO] Completed successfully.\n";
  } catch (const std::exception& exception) {
    std::cerr << "[PROFESSOR EDITABLE SCENARIO] Failure: "
              << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
