#include "MonteCarlo/BSEuler1D.h"
#include "MonteCarlo/BSMilstein1D.h"
#include "MonteCarlo/BSMilstein2D.h"
#include "MonteCarlo/SinglePath.h"
#include "SDE/Brownian1D.h"
#include "SDE/BrownianND.h"
#include "SDE/Heston.h"
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

struct Moments {
  double mean;
  double variance;
};

Moments ComputeMoments(const std::vector<double>& values) {
  Check(!values.empty(), "Moment computation requires at least one value");

  double sum = 0.0;
  for (double value : values) {
    sum += value;
  }

  const double mean = sum / static_cast<double>(values.size());
  double centeredSum = 0.0;
  for (double value : values) {
    const double centered = value - mean;
    centeredSum += centered * centered;
  }

  const double variance = centeredSum / static_cast<double>(values.size() - 1);
  return {mean, variance};
}

double ComputeCorrelation(const std::vector<double>& first,
                          const std::vector<double>& second) {
  Check(first.size() == second.size(), "Correlation requires matching samples");
  Check(first.size() > 1, "Correlation requires at least two samples");

  const Moments firstMoments = ComputeMoments(first);
  const Moments secondMoments = ComputeMoments(second);

  double covariance = 0.0;
  for (size_t index = 0; index < first.size(); ++index) {
    covariance += (first[index] - firstMoments.mean) *
                  (second[index] - secondMoments.mean);
  }
  covariance /= static_cast<double>(first.size() - 1);

  const double denominator = std::sqrt(firstMoments.variance * secondMoments.variance);
  Check(denominator > 0.0, "Correlation denominator must be positive");
  return covariance / denominator;
}

void PrintSection(const std::string& title) {
  std::cout << "[SDE TEST] " << title << std::endl;
}

void TestBrownian1D() {
  PrintSection("Brownian1D");

  EcuyerCombined generator(12345, 67890);
  Brownian1D process(&generator);

  const size_t sampleCount = 2000;
  const double startTime = 0.0;
  const double endTime = 1.0;
  const size_t nbSteps = 100;

  std::vector<double> terminalValues;
  terminalValues.reserve(sampleCount);
  for (size_t sample = 0; sample < sampleCount; ++sample) {
    process.Simulate(startTime, endTime, nbSteps);
    const SinglePath* path = process.GetPath(0);
    Check(path != nullptr, "Brownian1D path is null");
    terminalValues.push_back(path->GetState(endTime));
  }

  const Moments moments = ComputeMoments(terminalValues);
  Check(std::fabs(moments.mean) < 0.08, "Brownian1D mean is outside tolerance");
  Check(std::fabs(moments.variance - 1.0) < 0.12,
        "Brownian1D variance is outside tolerance");
}

void TestBrownianND() {
  PrintSection("BrownianND");

  EcuyerCombined generator(12345, 67890);
  const double rho = 0.4;
  std::vector<std::vector<double>> loadingMatrix = {
      {1.0, 0.0},
      {rho, std::sqrt(1.0 - rho * rho)},
  };
  BrownianND process(&generator, 2, &loadingMatrix);

  const size_t sampleCount = 2500;
  const double startTime = 0.0;
  const double endTime = 1.0;
  const size_t nbSteps = 100;

  std::vector<double> firstTerminalValues;
  std::vector<double> secondTerminalValues;
  firstTerminalValues.reserve(sampleCount);
  secondTerminalValues.reserve(sampleCount);

  for (size_t sample = 0; sample < sampleCount; ++sample) {
    process.Simulate(startTime, endTime, nbSteps);
    const SinglePath* firstPath = process.GetPath(0);
    const SinglePath* secondPath = process.GetPath(1);
    Check(firstPath != nullptr && secondPath != nullptr, "BrownianND paths are null");
    firstTerminalValues.push_back(firstPath->GetState(endTime));
    secondTerminalValues.push_back(secondPath->GetState(endTime));
  }

  const Moments firstMoments = ComputeMoments(firstTerminalValues);
  const Moments secondMoments = ComputeMoments(secondTerminalValues);
  const double empiricalCorrelation = ComputeCorrelation(firstTerminalValues, secondTerminalValues);

  Check(std::fabs(firstMoments.mean) < 0.08, "BrownianND first mean is outside tolerance");
  Check(std::fabs(secondMoments.mean) < 0.08, "BrownianND second mean is outside tolerance");
  Check(std::fabs(firstMoments.variance - 1.0) < 0.12,
        "BrownianND first variance is outside tolerance");
  Check(std::fabs(secondMoments.variance - 1.0) < 0.12,
        "BrownianND second variance is outside tolerance");
  Check(std::fabs(empiricalCorrelation - rho) < 0.05,
        "BrownianND correlation is outside tolerance");
}

void TestBSEuler1D() {
  PrintSection("BSEuler1D");

  EcuyerCombined generator(12345, 67890);
  const double spot0 = 100.0;
  const double rate = 0.05;
  const double vol = 0.2;
  const double maturity = 1.0;
  const size_t nbSteps = 100;
  const size_t sampleCount = 3000;

  BSEuler1D process(&generator, spot0, rate, vol);
  std::vector<double> terminalValues;
  terminalValues.reserve(sampleCount);
  for (size_t sample = 0; sample < sampleCount; ++sample) {
    process.Simulate(0.0, maturity, nbSteps);
    const SinglePath* path = process.GetPath(0);
    Check(path != nullptr, "BSEuler1D path is null");
    terminalValues.push_back(path->GetState(maturity));
  }

  const Moments moments = ComputeMoments(terminalValues);
  const double expectedMean = spot0 * std::exp(rate * maturity);
  Check(std::fabs(moments.mean - expectedMean) < 2.0,
        "BSEuler1D terminal mean is outside tolerance");
  Check(moments.variance > 0.0, "BSEuler1D terminal variance must be positive");
}

void TestBSMilstein1D() {
  PrintSection("BSMilstein1D");

  EcuyerCombined generator(12345, 67890);
  const double spot0 = 100.0;
  const double rate = 0.05;
  const double vol = 0.2;
  const double maturity = 1.0;
  const size_t nbSteps = 100;
  const size_t sampleCount = 3000;

  BSMilstein1D process(&generator, spot0, rate, vol);
  std::vector<double> terminalValues;
  terminalValues.reserve(sampleCount);
  for (size_t sample = 0; sample < sampleCount; ++sample) {
    process.Simulate(0.0, maturity, nbSteps);
    const SinglePath* path = process.GetPath(0);
    Check(path != nullptr, "BSMilstein1D path is null");
    terminalValues.push_back(path->GetState(maturity));
  }

  const Moments moments = ComputeMoments(terminalValues);
  const double expectedMean = spot0 * std::exp(rate * maturity);
  Check(std::fabs(moments.mean - expectedMean) < 1.5,
        "BSMilstein1D terminal mean is outside tolerance");
  Check(moments.variance > 0.0, "BSMilstein1D terminal variance must be positive");
}

void TestBSMilstein2D() {
  PrintSection("BSMilstein2D");

  EcuyerCombined generator(12345, 67890);
  const double spot1 = 100.0;
  const double spot2 = 120.0;
  const double rate1 = 0.03;
  const double rate2 = 0.04;
  const double vol1 = 0.2;
  const double vol2 = 0.25;
  const double rho = 0.35;
  const double maturity = 1.0;
  const size_t nbSteps = 100;
  const size_t sampleCount = 2500;

  BSMilstein2D process(&generator, spot1, spot2, rate1, rate2, vol1, vol2, rho);
  std::vector<double> firstTerminalValues;
  std::vector<double> secondTerminalValues;
  std::vector<double> firstLogReturns;
  std::vector<double> secondLogReturns;
  firstTerminalValues.reserve(sampleCount);
  secondTerminalValues.reserve(sampleCount);
  firstLogReturns.reserve(sampleCount);
  secondLogReturns.reserve(sampleCount);

  for (size_t sample = 0; sample < sampleCount; ++sample) {
    process.Simulate(0.0, maturity, nbSteps);
    const SinglePath* firstPath = process.GetPath(0);
    const SinglePath* secondPath = process.GetPath(1);
    Check(firstPath != nullptr && secondPath != nullptr, "BSMilstein2D paths are null");
    const double terminal1 = firstPath->GetState(maturity);
    const double terminal2 = secondPath->GetState(maturity);
    firstTerminalValues.push_back(terminal1);
    secondTerminalValues.push_back(terminal2);
    firstLogReturns.push_back(std::log(terminal1 / spot1));
    secondLogReturns.push_back(std::log(terminal2 / spot2));
  }

  const Moments firstMoments = ComputeMoments(firstTerminalValues);
  const Moments secondMoments = ComputeMoments(secondTerminalValues);
  const double empiricalCorrelation = ComputeCorrelation(firstLogReturns, secondLogReturns);
  const double expectedMean1 = spot1 * std::exp(rate1 * maturity);
  const double expectedMean2 = spot2 * std::exp(rate2 * maturity);

  Check(std::fabs(firstMoments.mean - expectedMean1) < 2.0,
        "BSMilstein2D first terminal mean is outside tolerance");
  Check(std::fabs(secondMoments.mean - expectedMean2) < 2.5,
        "BSMilstein2D second terminal mean is outside tolerance");
  Check(std::fabs(empiricalCorrelation - rho) < 0.08,
        "BSMilstein2D correlation is outside tolerance");
}

void TestHeston() {
  PrintSection("Heston");

  EcuyerCombined generator(12345, 67890);
  Heston process(&generator, 100.0, 0.04, 0.05, 0.04, 2.0, 0.3, -0.5);

  const size_t sampleCount = 300;
  const double maturity = 1.0;
  const size_t nbSteps = 100;

  double terminalVarianceSum = 0.0;
  for (size_t sample = 0; sample < sampleCount; ++sample) {
    process.Simulate(0.0, maturity, nbSteps);
    const SinglePath* spotPath = process.GetPath(0);
    const SinglePath* variancePath = process.GetPath(1);
    Check(spotPath != nullptr && variancePath != nullptr, "Heston paths are null");

    const std::vector<double> varianceValues = variancePath->GetAllValues();
    Check(!varianceValues.empty(), "Heston variance path is empty");
    for (double varianceValue : varianceValues) {
      Check(std::isfinite(varianceValue), "Heston variance must remain finite");
      Check(varianceValue >= 0.0, "Heston variance must remain non-negative");
    }

    const double terminalSpot = spotPath->GetState(maturity);
    const double terminalVariance = variancePath->GetState(maturity);
    Check(std::isfinite(terminalSpot), "Heston terminal spot must be finite");
    Check(terminalSpot > 0.0, "Heston terminal spot must stay positive");
    Check(std::isfinite(terminalVariance), "Heston terminal variance must be finite");
    Check(terminalVariance >= 0.0, "Heston terminal variance must be non-negative");
    terminalVarianceSum += terminalVariance;
  }

  const double averageTerminalVariance = terminalVarianceSum / static_cast<double>(sampleCount);
  Check(averageTerminalVariance >= 0.0,
        "Heston average terminal variance must be non-negative");
}

void TestValidationGuards() {
  PrintSection("Validation guards");

  EcuyerCombined generator(12345, 67890);

  ExpectThrows("SinglePath must reject empty or degenerate grids", []() {
    SinglePath invalidPath(1.0, 1.0, 10);
    (void)invalidPath;
  });

  ExpectThrows("SinglePath must reject zero step count", []() {
    SinglePath invalidPath(0.0, 1.0, 0);
    (void)invalidPath;
  });

  Brownian1D brownian(&generator);
  ExpectThrows("Brownian1D must reject degenerate time grids", [&brownian]() {
    brownian.Simulate(1.0, 1.0, 10);
  });
  ExpectThrows("Brownian1D must reject zero steps", [&brownian]() {
    brownian.Simulate(0.0, 1.0, 0);
  });

  std::vector<std::vector<double>> badLoadingMatrix = {{1.0, 0.0}};
  BrownianND brownianND(&generator, 2, &badLoadingMatrix);
  ExpectThrows("BrownianND must reject non-square loading matrices", [&brownianND]() {
    brownianND.Simulate(0.0, 1.0, 10);
  });

  ExpectThrows("BlackScholes1D must reject non-positive spot or negative volatility", [&generator]() {
    BSEuler1D invalidModel(&generator, 0.0, 0.05, 0.2);
    (void)invalidModel;
  });
  ExpectThrows("BlackScholes1D must reject negative volatility", [&generator]() {
    BSMilstein1D invalidModel(&generator, 100.0, 0.05, -0.2);
    (void)invalidModel;
  });

  ExpectThrows("BlackScholes2D must reject invalid correlation", [&generator]() {
    BSMilstein2D invalidModel(&generator, 100.0, 120.0, 0.03, 0.04, 0.2, 0.25, 1.5);
    (void)invalidModel;
  });

  ExpectThrows("Heston must reject invalid parameters", [&generator]() {
    Heston invalidModel(&generator, 100.0, -0.04, 0.05, 0.04, 2.0, 0.3, -0.5);
    (void)invalidModel;
  });
}

} // namespace

int main() {
  try {
    std::cout << "============================================================\n";
    std::cout << "  SDE / MONTE CARLO TEST SUITE \n";
    std::cout << "============================================================\n\n";

    TestBrownian1D();
    TestBrownianND();
    TestBSEuler1D();
    TestBSMilstein1D();
    TestBSMilstein2D();
    TestHeston();
    TestValidationGuards();

    std::cout << "\n[SDE TEST] All checks passed." << std::endl;
  } catch (const std::exception& exception) {
    std::cerr << "[SDE TEST] Failure: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
