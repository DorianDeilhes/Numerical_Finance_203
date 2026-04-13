#include "Pricing/BermudanBasket.h"
#include "Pricing/EuropeanBasket.h"
#include "UniformGenerator/EcuyerCombined.h"

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
void ExpectThrows(const std::string& message, Callable&& callable) {
  bool threw = false;
  try {
    callable();
  } catch (const std::exception&) {
    threw = true;
  }
  Check(threw, message);
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
}

}  // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  BERMUDAN BASKET TEST SUITE\n";
    std::cout << "============================================================\n\n";

    TestSingleExerciseMatchesEuropean();
    TestMoreExerciseDatesDoesNotDecreasePrice();
    TestValidationErrors();

    std::cout << "[BERMUDAN TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[BERMUDAN TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
