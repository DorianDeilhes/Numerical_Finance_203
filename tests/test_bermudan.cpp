#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/HaltonQuasiRandom.h"

#include <cmath>
#include <iostream>
#include <limits>
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
void ExpectThrows(const std::string& message, Callable&& callable) {
  bool threw = false;
  try {
    callable();
  } catch (const std::exception&) {
    threw = true;
  }
  Check(threw, message);
}

void CheckFinite(const MonteCarloSummary& summary, const std::string& label) {
  Check(std::isfinite(summary.mean), label + " mean must be finite");
  Check(std::isfinite(summary.sampleVariance), label + " variance must be finite");
  Check(std::isfinite(summary.standardError), label + " standard error must be finite");
  Check(summary.sampleSize >= 2, label + " sample size must be >= 2");
}

BermudanBasket BuildReferenceBermudan(size_t nb_steps = 60) {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.2, 0.25};
  std::vector<double> weights = {0.6, 0.4};
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};
  std::vector<double> exercise_dates = {0.0, 0.25, 0.5, 0.75, 1.0};

  return BermudanBasket(spot, vol, weights, strike, maturity, rate, corr,
                        exercise_dates, nb_steps);
}

void TestSingleExerciseMatchesEuropean() {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.2, 0.25};
  std::vector<double> weights = {0.6, 0.4};
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  EuropeanBasket european(spot, vol, weights, strike, maturity, rate, corr, 100);
  BermudanBasket bermudan(spot, vol, weights, strike, maturity, rate, corr,
                          /*exercise_dates=*/{maturity}, /*nb_steps=*/100);

  EcuyerCombined rng_eu(12345, 67890);
  EcuyerCombined rng_be(12345, 67890);

  const MonteCarloSummary eu = european.PriceFixedN(&rng_eu, 3000);
  const MonteCarloSummary be = bermudan.PriceFixedN(&rng_be, 3000);

  Check(std::fabs(eu.mean - be.mean) < 1e-10,
        "Bermudan with one exercise date at maturity must match European price");
}

void TestSingleExerciseWithDividendMatchesEuropean() {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.2, 0.25};
  std::vector<double> weights = {0.6, 0.4};
  std::vector<double> dividends = {0.06, 0.02};
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  EuropeanBasket european(spot, vol, weights, strike, maturity, rate, corr,
                          100, dividends);
  BermudanBasket bermudan(spot, vol, weights, strike, maturity, rate, corr,
                          /*exercise_dates=*/{maturity}, /*nb_steps=*/100,
                          dividends);

  EcuyerCombined rng_eu(22222, 33333);
  EcuyerCombined rng_be(22222, 33333);

  const MonteCarloSummary eu = european.PriceFixedN(&rng_eu, 3000);
  const MonteCarloSummary be = bermudan.PriceFixedN(&rng_be, 3000);

  Check(std::fabs(eu.mean - be.mean) < 1e-10,
        "Dividend-paying Bermudan with only maturity exercise must match European price");
}

void TestDividendCreatesEarlyExercisePremium() {
  const double spot = 100.0;
  const double volatility = 0.2;
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  const double dividend_yield = 0.10;
  const std::vector<std::vector<double>> corr = {{1.0}};
  const std::vector<double> exercise_dates = {0.0, 0.25, 0.5, 0.75, 1.0};

  EuropeanBasket european({spot}, {volatility}, {1.0}, strike, maturity, rate,
                          corr, 4, {dividend_yield});
  BermudanBasket bermudan({spot}, {volatility}, {1.0}, strike, maturity, rate,
                          corr, exercise_dates, 4, {dividend_yield});

  EcuyerCombined rng_eu(44444, 55555);
  EcuyerCombined rng_be(66666, 77777);

  const MonteCarloSummary eu = european.PriceFixedN(&rng_eu, 12000);
  const MonteCarloSummary be = bermudan.PriceFixedN(&rng_be, 12000);
  const double combined_se = std::sqrt(eu.sampleVariance / eu.sampleSize +
                                       be.sampleVariance / be.sampleSize);

  Check(be.mean > eu.mean + 2.0 * combined_se,
        "High dividend yield should create a visible Bermudan early-exercise premium");
}

void TestMoreExerciseDatesDoesNotDecreasePrice() {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.2, 0.25};
  std::vector<double> weights = {0.6, 0.4};
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  BermudanBasket sparse(spot, vol, weights, strike, maturity, rate, corr,
                        /*exercise_dates=*/{0.5, 1.0}, /*nb_steps=*/100);
  BermudanBasket dense(spot, vol, weights, strike, maturity, rate, corr,
                       /*exercise_dates=*/{0.25, 0.5, 0.75, 1.0}, /*nb_steps=*/100);

  EcuyerCombined rng_sparse(24680, 13579);
  EcuyerCombined rng_dense(24680, 13579);

  const MonteCarloSummary sparse_summary = sparse.PriceFixedN(&rng_sparse, 12000);
  const MonteCarloSummary dense_summary = dense.PriceFixedN(&rng_dense, 12000);

  // Monotonicity should hold in expectation. With finite Monte Carlo noise,
  // assert that confidence intervals are not clearly ordered in the wrong direction.
  Check(dense_summary.confidenceInterval.upper + 1e-12 >= sparse_summary.confidenceInterval.lower,
        "Adding Bermudan exercise dates should not reduce price (beyond Monte Carlo noise)");
}

void TestExerciseScheduleCanStartAtZero() {
  std::vector<double> spot = {100.0, 105.0};
  std::vector<double> vol = {0.2, 0.25};
  std::vector<double> weights = {0.6, 0.4};
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.35}, {0.35, 1.0}};

  BermudanBasket bermudan(spot, vol, weights, strike, maturity, rate, corr,
                          /*exercise_dates=*/{0.0, 0.5, 1.0}, /*nb_steps=*/100);
  EcuyerCombined rng(31415, 27182);

  const MonteCarloSummary summary = bermudan.PriceFixedN(&rng, 2000);
  Check(std::isfinite(summary.mean), "Bermudan schedule starting at t0 must produce finite price");
  Check(summary.sampleSize == 2000, "Bermudan schedule starting at t0 must use requested path count");
}

void TestPhase5QuasiRandomRuns() {
  BermudanBasket bermudan = BuildReferenceBermudan();
  HaltonQuasiRandom halton(/*dimension=*/128, /*use shift=*/true, /*seed=*/0.314159);

  const MonteCarloSummary summary = bermudan.PriceFixedN(&halton, 700);
  CheckFinite(summary, "Bermudan quasi-random");
  Check(summary.mean > 0.0, "Bermudan quasi-random price must be positive");
}

void TestPhase5ControlVariateRuns() {
  BermudanBasket bermudan = BuildReferenceBermudan();
  EcuyerCombined rng(13579, 24680);

  const MonteCarloSummary summary = bermudan.PriceFixedNControlVariate(&rng, 700, 150);
  CheckFinite(summary, "Bermudan control variate");
  Check(summary.sampleSize == 700, "Bermudan control variate must use requested path count");
}

void TestPhase5AntitheticRuns() {
  BermudanBasket bermudan = BuildReferenceBermudan();
  EcuyerCombined rng(98765, 43210);

  const MonteCarloSummary summary = bermudan.PriceFixedNAntithetic(&rng, 500);
  CheckFinite(summary, "Bermudan antithetic");
  Check(summary.sampleSize == 500, "Bermudan antithetic must use requested pair count");
}

void TestPhase5CumulativeRuns() {
  BermudanBasket bermudan = BuildReferenceBermudan();
  HaltonQuasiRandom halton(/*dimension=*/128, /*use shift=*/true, /*seed=*/0.271828);

  const MonteCarloSummary summary = bermudan.PriceFixedNCumulative(&halton, 450, 120);
  CheckFinite(summary, "Bermudan cumulative");
  Check(summary.sampleSize == 450, "Bermudan cumulative must use requested pair count");
}

void TestValidationErrors() {
  std::vector<double> spot = {100.0, 100.0};
  std::vector<double> vol = {0.2, 0.2};
  std::vector<double> weights = {0.5, 0.5};
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.03;
  std::vector<std::vector<double>> corr = {{1.0, 0.2}, {0.2, 1.0}};

  ExpectThrows("Should reject empty exercise dates", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr, {}, 100);
  });

  ExpectThrows("Should reject non-increasing exercise dates", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr, {0.5, 0.5, 1.0}, 100);
  });

  ExpectThrows("Should reject last exercise date different from maturity", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr, {0.5, 0.9}, 100);
  });

  ExpectThrows("Should reject exercise dates not aligned with nb_steps grid", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr, {0.3333, 1.0}, 100);
    EcuyerCombined rng(10101, 20202);
    (void)basket.PriceFixedN(&rng, 10);
  });

  ExpectThrows("Should reject nb_steps == 0", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr, {0.5, 1.0}, 0);
  });

  ExpectThrows("Should reject NaN spot price", [&]() {
    BermudanBasket basket({std::numeric_limits<double>::quiet_NaN(), 100.0}, vol,
                          weights, strike, maturity, rate, corr, {0.5, 1.0}, 100);
  });

  ExpectThrows("Should reject infinite volatility", [&]() {
    BermudanBasket basket(spot, {std::numeric_limits<double>::infinity(), 0.2},
                          weights, strike, maturity, rate, corr, {0.5, 1.0}, 100);
  });

  ExpectThrows("Should reject NaN weight", [&]() {
    BermudanBasket basket(spot, vol, {std::numeric_limits<double>::quiet_NaN(), 0.5},
                          strike, maturity, rate, corr, {0.5, 1.0}, 100);
  });

  ExpectThrows("Should reject infinite strike", [&]() {
    BermudanBasket basket(spot, vol, weights, std::numeric_limits<double>::infinity(),
                          maturity, rate, corr, {0.5, 1.0}, 100);
  });

  ExpectThrows("Should reject NaN exercise date", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr,
                          {std::numeric_limits<double>::quiet_NaN(), 1.0}, 100);
  });

  ExpectThrows("Should reject non-square correlation matrix", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate,
                          {{1.0}, {0.2, 1.0}}, {0.5, 1.0}, 100);
  });

  ExpectThrows("Should reject dividend yield size mismatch", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr,
                          {0.5, 1.0}, 100, {0.01});
  });

  ExpectThrows("Should reject negative dividend yield", [&]() {
    BermudanBasket basket(spot, vol, weights, strike, maturity, rate, corr,
                          {0.5, 1.0}, 100, {0.01, -0.02});
  });
}

void TestPhase5ValidationErrors() {
  BermudanBasket bermudan = BuildReferenceBermudan();
  EcuyerCombined rng(11111, 22222);

  ExpectThrows("Should reject null generator in control variate", [&]() {
    (void)bermudan.PriceFixedNControlVariate(nullptr, 20, 10);
  });

  ExpectThrows("Should reject null generator in PriceFixedN", [&]() {
    (void)bermudan.PriceFixedN(nullptr, 20);
  });

  ExpectThrows("Should reject path_count < 2 in PriceFixedN", [&]() {
    (void)bermudan.PriceFixedN(&rng, 1);
  });

  ExpectThrows("Should reject path_count < 2 in control variate", [&]() {
    (void)bermudan.PriceFixedNControlVariate(&rng, 1, 10);
  });

  ExpectThrows("Should reject pilot_count < 2 in control variate", [&]() {
    (void)bermudan.PriceFixedNControlVariate(&rng, 20, 1);
  });

  ExpectThrows("Should reject pair_count < 2 in antithetic", [&]() {
    (void)bermudan.PriceFixedNAntithetic(&rng, 1);
  });

  ExpectThrows("Should reject pilot_count < 2 in cumulative", [&]() {
    (void)bermudan.PriceFixedNCumulative(&rng, 20, 1);
  });

  BermudanBasket negative_weight_bermudan({100.0, 105.0}, {0.2, 0.25},
                                          {0.7, -0.2}, 100.0, 1.0, 0.03,
                                          {{1.0, 0.35}, {0.35, 1.0}},
                                          {0.0, 0.5, 1.0}, 20);
  ExpectThrows("Bermudan control variate should reject negative weights", [&]() {
    EcuyerCombined local_rng(33333, 44444);
    (void)negative_weight_bermudan.PriceFixedNControlVariate(&local_rng, 10, 5);
  });

  BermudanBasket non_normalized_weight_bermudan({100.0, 105.0}, {0.2, 0.25},
                                                {0.7, 0.2}, 100.0, 1.0, 0.03,
                                                {{1.0, 0.35}, {0.35, 1.0}},
                                                {0.0, 0.5, 1.0}, 20);
  ExpectThrows("Bermudan control variate should reject weights not summing to 1", [&]() {
    EcuyerCombined local_rng(55555, 66666);
    (void)non_normalized_weight_bermudan.PriceFixedNCumulative(&local_rng, 10, 5);
  });
}

}  // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  BERMUDAN BASKET TEST SUITE\n";
    std::cout << "============================================================\n\n";

    TestSingleExerciseMatchesEuropean();
    TestSingleExerciseWithDividendMatchesEuropean();
    TestDividendCreatesEarlyExercisePremium();
    TestMoreExerciseDatesDoesNotDecreasePrice();
    TestExerciseScheduleCanStartAtZero();
    TestPhase5QuasiRandomRuns();
    TestPhase5ControlVariateRuns();
    TestPhase5AntitheticRuns();
    TestPhase5CumulativeRuns();
    TestValidationErrors();
    TestPhase5ValidationErrors();

    std::cout << "[BERMUDAN TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[BERMUDAN TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
