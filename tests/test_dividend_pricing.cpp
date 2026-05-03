#include "MonteCarlo/Helper/SimulateGeometricBrownianTerminalND.h"
#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "Pricing/Helper/BuildLoadingMatrixFromCorrelation.h"
#include "Pricing/Helper/DiscountedGeometricBasketCall.h"
#include "Pricing/Helper/KnownMeanDiscountedGeometricBasketCall.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/HaltonQuasiRandom.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void Check(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename Callable>
void ExpectThrows(const std::string& message, Callable callable) {
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
  Check(samples.size() >= 2, "ComputeSampleStats requires at least two samples");

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

  SampleStats stats;
  stats.mean = mean;
  stats.sample_variance = squared_sum / static_cast<double>(samples.size() - 1);
  stats.standard_error = std::sqrt(stats.sample_variance / static_cast<double>(samples.size()));
  return stats;
}

double NormalCDF(double x) {
  return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double BlackScholesCall(double spot,
                        double strike,
                        double maturity,
                        double rate,
                        double volatility,
                        double dividend_yield = 0.0) {
  if (volatility < 1e-12) {
    return std::max(spot * std::exp(-dividend_yield * maturity) -
                        strike * std::exp(-rate * maturity),
                    0.0);
  }

  const double sqrt_maturity = std::sqrt(maturity);
  const double d1 =
      (std::log(spot / strike) +
       (rate - dividend_yield + 0.5 * volatility * volatility) * maturity) /
      (volatility * sqrt_maturity);
  const double d2 = d1 - volatility * sqrt_maturity;

  return spot * std::exp(-dividend_yield * maturity) * NormalCDF(d1) -
         strike * std::exp(-rate * maturity) * NormalCDF(d2);
}

void TestZeroVolatilityPricing() {
  const double spot = 100.0;
  const double strike = 97.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  const double expected = std::max(spot - strike * std::exp(-rate * maturity), 0.0);

  EuropeanBasket pricer({spot}, {0.0}, {1.0}, strike, maturity, rate, {{1.0}}, 50);
  EcuyerCombined generator(1, 2);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 500);

  Check(std::abs(summary.mean - expected) < 0.05,
        "Zero-volatility European price should match deterministic value");
}

void TestOneAssetBasketMatchesBlackScholes() {
  const double spot = 100.0;
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.05;
  const double volatility = 0.20;
  const double expected = BlackScholesCall(spot, strike, maturity, rate, volatility);

  EuropeanBasket pricer({spot}, {volatility}, {1.0}, strike, maturity, rate, {{1.0}}, 100);
  EcuyerCombined generator(1111, 2222);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 5000);

  Check(std::abs(summary.mean - expected) < 3.0 * summary.standardError,
        "One-asset basket should match Black-Scholes formula");
}

void TestBermudanPriceAtLeastEuropean() {
  const std::vector<double> spots = {100.0, 105.0};
  const std::vector<double> volatilities = {0.20, 0.25};
  const std::vector<double> weights = {0.60, 0.40};
  const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  EuropeanBasket european(spots, volatilities, weights, 100.0, 1.0, 0.03, corr, 4);
  BermudanBasket bermudan(spots, volatilities, weights, 100.0, 1.0, 0.03, corr,
                          {0.0, 0.25, 0.50, 0.75, 1.0}, 4);
  EcuyerCombined gen_euro(11, 22);
  EcuyerCombined gen_berm(33, 44);
  const MonteCarloSummary euro = european.PriceFixedN(&gen_euro, 4000);
  const MonteCarloSummary berm = bermudan.PriceFixedN(&gen_berm, 4000);
  const double combined_se =
      std::sqrt(euro.sampleVariance / euro.sampleSize + berm.sampleVariance / berm.sampleSize);

  Check(berm.mean >= euro.mean - 3.0 * combined_se,
        "Bermudan price should not be materially below European price");
}

void TestBermudanMaturityOnlyMatchesEuropean() {
  const std::vector<double> spots = {100.0, 105.0};
  const std::vector<double> volatilities = {0.20, 0.25};
  const std::vector<double> weights = {0.60, 0.40};
  const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  EuropeanBasket european(spots, volatilities, weights, 100.0, 1.0, 0.03, corr, 4);
  BermudanBasket bermudan(spots, volatilities, weights, 100.0, 1.0, 0.03, corr,
                          {1.0}, 4);
  EcuyerCombined gen_euro(55, 66);
  EcuyerCombined gen_berm(55, 66);
  const MonteCarloSummary euro = european.PriceFixedN(&gen_euro, 3000);
  const MonteCarloSummary berm = bermudan.PriceFixedN(&gen_berm, 3000);

  Check(std::abs(berm.mean - euro.mean) < 1e-10,
        "Bermudan with only maturity exercise should match European with same paths");
}

void TestVarianceReductionOrdering() {
  const std::vector<double> spots = {100.0, 105.0};
  const std::vector<double> volatilities = {0.20, 0.25};
  const std::vector<double> weights = {0.60, 0.40};
  const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  EuropeanBasket pricer(spots, volatilities, weights, 100.0, 1.0, 0.03, corr, 4);
  EcuyerCombined gen_basic(11, 22);
  EcuyerCombined gen_antithetic(33, 44);
  EcuyerCombined gen_cumulative(55, 66);
  const MonteCarloSummary basic = pricer.PriceFixedN(&gen_basic, 3000);
  const MonteCarloSummary antithetic = pricer.PriceFixedNAntithetic(&gen_antithetic, 1500);
  const MonteCarloSummary cumulative = pricer.PriceFixedNCumulative(&gen_cumulative, 1500, 500);

  Check(antithetic.sampleVariance < basic.sampleVariance,
        "Antithetic variance should be below basic variance for this seeded scenario");
  Check(cumulative.sampleVariance < antithetic.sampleVariance,
        "Cumulative variance should be below antithetic variance for this seeded scenario");
}

void TestDividendCreatesEarlyExercisePremium() {
  const double spot = 100.0;
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  const double volatility = 0.20;
  const std::vector<std::vector<double>> corr = {{1.0}};
  const std::vector<double> dates = {0.0, 0.25, 0.50, 0.75, 1.0};

  EuropeanBasket european({spot}, {volatility}, {1.0}, strike, maturity, rate, corr, 4, {0.10});
  BermudanBasket bermudan({spot}, {volatility}, {1.0}, strike, maturity, rate, corr,
                          dates, 4, {0.10});
  EcuyerCombined gen_euro(111, 222);
  EcuyerCombined gen_berm(333, 444);
  const MonteCarloSummary euro = european.PriceFixedN(&gen_euro, 8000);
  const MonteCarloSummary berm = bermudan.PriceFixedN(&gen_berm, 8000);
  const double combined_se =
      std::sqrt(euro.sampleVariance / euro.sampleSize + berm.sampleVariance / berm.sampleSize);

  Check(berm.mean > euro.mean + 2.0 * combined_se,
        "High dividend yield should create a visible Bermudan premium");
}

void TestRateSensitivityWithoutDividendsCanRun() {
  const std::vector<double> spots = {100.0, 105.0};
  const std::vector<double> volatilities = {0.20, 0.25};
  const std::vector<double> weights = {0.60, 0.40};
  const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
  const std::vector<double> dates = {0.0, 0.25, 0.50, 0.75, 1.0};

  const double rates[] = {0.05, 0.03, 0.01, 0.00};
  for (size_t i = 0; i < 4; ++i) {
    EuropeanBasket european(spots, volatilities, weights, 100.0, 1.0, rates[i], corr, 4);
    BermudanBasket bermudan(spots, volatilities, weights, 100.0, 1.0, rates[i], corr, dates, 4);
    EcuyerCombined gen_euro(11, 22);
    EcuyerCombined gen_berm(33, 44);
    CheckFiniteSummary(european.PriceFixedN(&gen_euro, 1000), "European rate sensitivity");
    CheckFiniteSummary(bermudan.PriceFixedN(&gen_berm, 1000), "Bermudan rate sensitivity");
  }
}

void TestIdentityCorrelation() {
  EuropeanBasket pricer({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 0.0}, {0.0, 1.0}}, 4);
  EcuyerCombined generator(101, 202);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 4000);
  CheckFiniteSummary(summary, "Identity correlation");
  Check(summary.mean > 0.0, "Identity correlation should produce a positive price");
}

void TestPerfectCorrelation() {
  EuropeanBasket pricer({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 1.0}, {1.0, 1.0}}, 4);
  EcuyerCombined generator(303, 404);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 4000);
  CheckFiniteSummary(summary, "Perfect correlation");
  Check(summary.mean > 0.0, "Perfect correlation should produce a positive price");
}

void TestZeroWeightAsset() {
  const double expected = BlackScholesCall(100.0, 100.0, 1.0, 0.05, 0.20);
  EuropeanBasket pricer({100.0, 150.0}, {0.20, 0.50}, {1.0, 0.0},
                        100.0, 1.0, 0.05, {{1.0, 0.0}, {0.0, 1.0}}, 100);
  EcuyerCombined generator(505, 606);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 6000);
  Check(std::abs(summary.mean - expected) < 3.0 * summary.standardError,
        "Zero-weight asset should not affect the one-asset basket price");
}

void TestLargeSampleRun() {
  const size_t sample_count = 50000;
  EuropeanBasket pricer({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}}, 4);
  EcuyerCombined generator(707, 808);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, sample_count);
  CheckFiniteSummary(summary, "Large sample run");
  Check(summary.sampleSize == sample_count, "Large sample run should keep requested sample size");
}

void TestNegativeWeightsEuropean() {
  EuropeanBasket pricer({100.0, 90.0}, {0.20, 0.20}, {1.20, -0.20},
                        100.0, 1.0, 0.03, {{1.0, 0.30}, {0.30, 1.0}}, 4);
  EcuyerCombined generator(909, 1001);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 5000);
  CheckFiniteSummary(summary, "Negative weights");
  Check(summary.mean >= 0.0, "European pricing with negative weights should stay non-negative");
}

void TestZeroStrike() {
  EuropeanBasket pricer({100.0}, {0.20}, {1.0}, 0.0, 1.0, 0.05, {{1.0}}, 100);
  EcuyerCombined generator(1102, 1203);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 5000);
  Check(std::abs(summary.mean - 100.0) < 3.0 * summary.standardError,
        "Zero-strike one-asset call should price near S0 when q=0");
}

void TestDeterministicWorld() {
  EuropeanBasket pricer({100.0}, {0.0}, {1.0}, 97.0, 1.0, 0.0, {{1.0}}, 20);
  EcuyerCombined generator(1304, 1405);
  const MonteCarloSummary summary = pricer.PriceFixedN(&generator, 200);
  Check(std::abs(summary.mean - 3.0) < 1e-12 && summary.sampleVariance == 0.0,
        "Deterministic world should give deterministic payoff");
}

void TestExerciseDateAtZero() {
  BermudanBasket with_t0({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                         {{1.0}}, {0.0, 0.5, 1.0}, 4, {0.10});
  BermudanBasket without_t0({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                            {{1.0}}, {0.5, 1.0}, 4, {0.10});
  EcuyerCombined gen_with(1506, 1607);
  EcuyerCombined gen_without(1708, 1809);
  const MonteCarloSummary s1 = with_t0.PriceFixedN(&gen_with, 6000);
  const MonteCarloSummary s2 = without_t0.PriceFixedN(&gen_without, 6000);
  const double combined_se =
      std::sqrt(s1.sampleVariance / s1.sampleSize + s2.sampleVariance / s2.sampleSize);
  Check(s1.mean >= s2.mean - 3.0 * combined_se,
        "Exercise date t0=0 should not materially reduce Bermudan value");
}

void TestInvalidInputRejection() {
  ExpectThrows("Negative dividend yield should throw", []() {
    EuropeanBasket pricer({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                          {{1.0}}, 4, {-0.01});
  });

  ExpectThrows("Misaligned exercise date should throw on pricing", []() {
    BermudanBasket pricer({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                          {{1.0}}, {0.3333, 1.0}, 4);
    EcuyerCombined generator(1910, 2011);
    (void)pricer.PriceFixedN(&generator, 10);
  });
}

void TestDividendAntitheticPath() {
  EuropeanBasket low_q({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                       {{1.0}}, 40, {0.00});
  EuropeanBasket high_q({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                        {{1.0}}, 40, {0.08});
  EcuyerCombined gen_low(2101, 2202);
  EcuyerCombined gen_high(2303, 2404);
  const MonteCarloSummary low = low_q.PriceFixedNAntithetic(&gen_low, 3000);
  const MonteCarloSummary high = high_q.PriceFixedNAntithetic(&gen_high, 3000);
  const double combined_se =
      std::sqrt(low.sampleVariance / low.sampleSize + high.sampleVariance / high.sampleSize);
  CheckFiniteSummary(low, "Dividend antithetic low-q");
  CheckFiniteSummary(high, "Dividend antithetic high-q");
  Check(high.mean < low.mean - 2.0 * combined_se,
        "Higher dividend should reduce European value under antithetic pricing");
}

void TestDividendControlVariatePath() {
  EuropeanBasket low_q({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                       100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}},
                       20, {0.00, 0.00});
  EuropeanBasket high_q({100.0, 105.0}, {0.20, 0.25}, {0.60, 0.40},
                        100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}},
                        20, {0.08, 0.05});
  EcuyerCombined gen_low(2505, 2606);
  EcuyerCombined gen_high(2707, 2808);
  const MonteCarloSummary low = low_q.PriceFixedNControlVariate(&gen_low, 3000, 700);
  const MonteCarloSummary high = high_q.PriceFixedNControlVariate(&gen_high, 3000, 700);
  const double combined_se =
      std::sqrt(low.sampleVariance / low.sampleSize + high.sampleVariance / high.sampleSize);
  CheckFiniteSummary(low, "Dividend control variate low-q");
  CheckFiniteSummary(high, "Dividend control variate high-q");
  Check(high.mean < low.mean - 2.0 * combined_se,
        "Higher dividend should reduce European value under control variate pricing");
}

void TestDividendCumulativePath() {
  EuropeanBasket european({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                          {{1.0}}, 20, {0.10});
  BermudanBasket bermudan({100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.03,
                          {{1.0}}, {0.0, 0.25, 0.50, 0.75, 1.0}, 20, {0.10});
  HaltonQuasiRandom gen_euro(20, true, 0.123456);
  HaltonQuasiRandom gen_berm(20, true, 0.654321);
  const MonteCarloSummary euro = european.PriceFixedNCumulative(&gen_euro, 2500, 700);
  const MonteCarloSummary berm = bermudan.PriceFixedNCumulative(&gen_berm, 2500, 700);
  const double combined_se =
      std::sqrt(euro.sampleVariance / euro.sampleSize + berm.sampleVariance / berm.sampleSize);
  CheckFiniteSummary(euro, "Dividend cumulative European");
  CheckFiniteSummary(berm, "Dividend cumulative Bermudan");
  Check(berm.mean > euro.mean + 2.0 * combined_se,
        "High dividend should create Bermudan premium under cumulative pricing");
}

void TestKnownMeanOneDimensionalDividend() {
  const double helper = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
      {100.0}, {0.20}, {1.0}, 100.0, 1.0, 0.05, {0.04}, {{1.0}});
  const double expected = BlackScholesCall(100.0, 100.0, 1.0, 0.05, 0.20, 0.04);
  Check(std::abs(helper - expected) < 1e-12,
        "Geometric control mean should match 1D Black-Scholes-Merton");
}

void TestKnownMeanTwoDimensionalDividendAgainstMonteCarlo() {
  const std::vector<double> spots = {100.0, 105.0};
  const std::vector<double> volatilities = {0.20, 0.25};
  const std::vector<double> weights = {0.60, 0.40};
  const std::vector<double> dividends = {0.04, 0.02};
  const std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
  const double analytic = PricingHelper::KnownMeanDiscountedGeometricBasketCall(
      spots, volatilities, weights, 100.0, 1.0, 0.03, dividends, corr);

  std::vector<std::vector<double>> loading =
      PricingHelper::BuildLoadingMatrixFromCorrelation(corr);
  EcuyerCombined generator(2909, 3010);
  std::vector<double> samples;
  const size_t sample_count = 8000;
  samples.reserve(sample_count);
  for (size_t i = 0; i < sample_count; ++i) {
    const std::vector<double> terminal =
        MonteCarloHelper::SimulateGeometricBrownianTerminalND(
            &generator, spots, volatilities, 0.03, dividends, &loading, 0.0, 1.0, 20);
    samples.push_back(PricingHelper::DiscountedGeometricBasketCall(
        terminal, weights, 100.0, 0.03, 1.0));
  }

  const SampleStats stats = ComputeSampleStats(samples);
  Check(std::abs(stats.mean - analytic) < 3.0 * stats.standard_error,
        "Geometric control mean should match direct Monte Carlo estimate");
}

void TestStrongImmediateExerciseAtZero() {
  BermudanBasket with_t0({120.0}, {0.0}, {1.0}, 100.0, 1.0, 0.03,
                         {{1.0}}, {0.0, 0.25, 0.50, 0.75, 1.0}, 20, {0.20});
  BermudanBasket without_t0({120.0}, {0.0}, {1.0}, 100.0, 1.0, 0.03,
                            {{1.0}}, {0.25, 0.50, 0.75, 1.0}, 20, {0.20});
  EcuyerCombined gen_with(3111, 3212);
  EcuyerCombined gen_without(3313, 3414);
  const MonteCarloSummary s1 = with_t0.PriceFixedN(&gen_with, 200);
  const MonteCarloSummary s2 = without_t0.PriceFixedN(&gen_without, 200);
  Check(std::abs(s1.mean - 20.0) < 1e-12 && s1.mean > s2.mean + 1.0,
        "Strong t0 scenario should exercise immediately");
}

void TestMultiAssetDividendSensitivity() {
  const std::vector<double> spots = {120.0, 110.0};
  const std::vector<double> volatilities = {0.0, 0.0};
  const std::vector<double> weights = {0.50, 0.50};
  const std::vector<std::vector<double>> corr = {{1.0, 0.0}, {0.0, 1.0}};
  const std::vector<double> dates = {0.25, 0.50, 0.75, 1.0};

  EuropeanBasket euro_low(spots, volatilities, weights, 100.0, 1.0, 0.03,
                          corr, 20, {0.00, 0.00});
  EuropeanBasket euro_high(spots, volatilities, weights, 100.0, 1.0, 0.03,
                           corr, 20, {0.20, 0.15});
  BermudanBasket berm_low(spots, volatilities, weights, 100.0, 1.0, 0.03,
                          corr, dates, 20, {0.00, 0.00});
  BermudanBasket berm_high(spots, volatilities, weights, 100.0, 1.0, 0.03,
                           corr, dates, 20, {0.20, 0.15});
  EcuyerCombined gen1(3515, 3616);
  EcuyerCombined gen2(3717, 3818);
  EcuyerCombined gen3(3919, 4020);
  EcuyerCombined gen4(4121, 4222);
  const MonteCarloSummary euro_low_summary = euro_low.PriceFixedN(&gen1, 200);
  const MonteCarloSummary euro_high_summary = euro_high.PriceFixedN(&gen2, 200);
  const MonteCarloSummary berm_low_summary = berm_low.PriceFixedN(&gen3, 200);
  const MonteCarloSummary berm_high_summary = berm_high.PriceFixedN(&gen4, 200);

  const double premium_low = berm_low_summary.mean - euro_low_summary.mean;
  const double premium_high = berm_high_summary.mean - euro_high_summary.mean;
  Check(euro_high_summary.mean < euro_low_summary.mean &&
            premium_high > premium_low + 1e-12,
        "Higher dividends should reduce European value and increase Bermudan premium");
}

void TestAllZeroWeights() {
  EuropeanBasket european({100.0, 120.0}, {0.20, 0.25}, {0.0, 0.0},
                          100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}}, 4);
  BermudanBasket bermudan({100.0, 120.0}, {0.20, 0.25}, {0.0, 0.0},
                          100.0, 1.0, 0.03, {{1.0, 0.35}, {0.35, 1.0}},
                          {0.0, 0.5, 1.0}, 4);
  EcuyerCombined gen_euro(4323, 4424);
  EcuyerCombined gen_berm(4525, 4626);
  const MonteCarloSummary euro = european.PriceFixedN(&gen_euro, 500);
  const MonteCarloSummary berm = bermudan.PriceFixedN(&gen_berm, 500);
  Check(std::abs(euro.mean) < 1e-12 && std::abs(berm.mean) < 1e-12 &&
            euro.sampleVariance == 0.0 && berm.sampleVariance == 0.0,
        "All-zero basket weights should produce zero prices");
}

void TestDeepInTheMoneyEuropean() {
  const double expected = BlackScholesCall(200.0, 50.0, 1.0, 0.03, 0.20);
  EuropeanBasket european({200.0}, {0.20}, {1.0}, 50.0, 1.0, 0.03,
                          {{1.0}}, 40);
  EcuyerCombined generator(4727, 4828);
  const MonteCarloSummary summary = european.PriceFixedN(&generator, 4000);
  Check(std::abs(summary.mean - expected) < 3.0 * summary.standardError,
        "Deep ITM European call should match Black-Scholes");
}

void TestDeepOutOfTheMoneyEuropean() {
  const double expected = BlackScholesCall(70.0, 130.0, 1.0, 0.03, 0.20);
  EuropeanBasket european({70.0}, {0.20}, {1.0}, 130.0, 1.0, 0.03,
                          {{1.0}}, 40);
  EcuyerCombined generator(4929, 5030);
  const MonteCarloSummary summary = european.PriceFixedN(&generator, 12000);
  Check(summary.mean >= 0.0 && std::abs(summary.mean - expected) < 3.0 * summary.standardError,
        "Deep OTM European call should match Black-Scholes within Monte Carlo error");
}

void TestDeepInTheMoneyBermudanDividendPremium() {
  EuropeanBasket european({200.0}, {0.20}, {1.0}, 50.0, 1.0, 0.03,
                          {{1.0}}, 20, {0.12});
  BermudanBasket bermudan({200.0}, {0.20}, {1.0}, 50.0, 1.0, 0.03,
                          {{1.0}}, {0.25, 0.50, 0.75, 1.0}, 20, {0.12});
  EcuyerCombined gen_euro(5131, 5232);
  EcuyerCombined gen_berm(5333, 5434);
  const MonteCarloSummary euro = european.PriceFixedN(&gen_euro, 5000);
  const MonteCarloSummary berm = bermudan.PriceFixedN(&gen_berm, 5000);
  const double combined_se =
      std::sqrt(euro.sampleVariance / euro.sampleSize + berm.sampleVariance / berm.sampleSize);
  Check(berm.mean > euro.mean + 2.0 * combined_se,
        "Deep ITM Bermudan with dividends should have visible early-exercise premium");
}

void TestDeepOutOfTheMoneyBermudanVsEuropean() {
  EuropeanBasket european({60.0}, {0.20}, {1.0}, 140.0, 1.0, 0.03,
                          {{1.0}}, 20, {0.05});
  BermudanBasket bermudan({60.0}, {0.20}, {1.0}, 140.0, 1.0, 0.03,
                          {{1.0}}, {0.25, 0.50, 0.75, 1.0}, 20, {0.05});
  EcuyerCombined gen_euro(5535, 5636);
  EcuyerCombined gen_berm(5737, 5838);
  const MonteCarloSummary euro = european.PriceFixedN(&gen_euro, 12000);
  const MonteCarloSummary berm = bermudan.PriceFixedN(&gen_berm, 12000);
  const double combined_se =
      std::sqrt(euro.sampleVariance / euro.sampleSize + berm.sampleVariance / berm.sampleSize);
  Check(euro.mean >= 0.0 && berm.mean >= 0.0 &&
            std::abs(berm.mean - euro.mean) <= 3.0 * combined_se,
        "Deep OTM Bermudan and European prices should remain close");
}

void RunTest(const std::string& name, void (*test_function)()) {
  std::cout << "[test_dividend] " << name << "\n";
  test_function();
}

}  // namespace

int main() {
  try {
    RunTest("zero volatility pricing", TestZeroVolatilityPricing);
    RunTest("one-asset basket vs Black-Scholes", TestOneAssetBasketMatchesBlackScholes);
    RunTest("Bermudan price at least European", TestBermudanPriceAtLeastEuropean);
    RunTest("Bermudan maturity-only equals European", TestBermudanMaturityOnlyMatchesEuropean);
    RunTest("variance reduction ordering", TestVarianceReductionOrdering);
    RunTest("dividend early-exercise premium", TestDividendCreatesEarlyExercisePremium);
    RunTest("rate sensitivity without dividends", TestRateSensitivityWithoutDividendsCanRun);
    RunTest("identity correlation", TestIdentityCorrelation);
    RunTest("perfect correlation", TestPerfectCorrelation);
    RunTest("zero-weight asset", TestZeroWeightAsset);
    RunTest("large sample run", TestLargeSampleRun);
    RunTest("negative weights European", TestNegativeWeightsEuropean);
    RunTest("zero strike", TestZeroStrike);
    RunTest("deterministic world", TestDeterministicWorld);
    RunTest("exercise date at zero", TestExerciseDateAtZero);
    RunTest("invalid input rejection", TestInvalidInputRejection);
    RunTest("dividend antithetic path", TestDividendAntitheticPath);
    RunTest("dividend control variate path", TestDividendControlVariatePath);
    RunTest("dividend cumulative path", TestDividendCumulativePath);
    RunTest("known mean one-dimensional dividend", TestKnownMeanOneDimensionalDividend);
    RunTest("known mean two-dimensional dividend", TestKnownMeanTwoDimensionalDividendAgainstMonteCarlo);
    RunTest("strong immediate exercise at zero", TestStrongImmediateExerciseAtZero);
    RunTest("multi-asset dividend sensitivity", TestMultiAssetDividendSensitivity);
    RunTest("all-zero weights", TestAllZeroWeights);
    RunTest("deep ITM European", TestDeepInTheMoneyEuropean);
    RunTest("deep OTM European", TestDeepOutOfTheMoneyEuropean);
    RunTest("deep ITM Bermudan dividend premium", TestDeepInTheMoneyBermudanDividendPremium);
    RunTest("deep OTM Bermudan vs European", TestDeepOutOfTheMoneyBermudanVsEuropean);
  } catch (const std::exception& exception) {
    std::cerr << "[test_dividend] failure: " << exception.what() << "\n";
    return 1;
  }

  std::cout << "[test_dividend] all checks passed\n";
  return 0;
}
