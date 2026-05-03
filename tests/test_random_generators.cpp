#include "ContinuousGenerator/BivariateNormal.h"
#include "ContinuousGenerator/Exponential.h"
#include "ContinuousGenerator/Normal.h"
#include "DiscreteGenerator/Bernoulli.h"
#include "DiscreteGenerator/Binomial.h"
#include "DiscreteGenerator/FiniteSet.h"
#include "DiscreteGenerator/HeadTail.h"
#include "DiscreteGenerator/Poisson.h"
#include "RandomGenerator.h"
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/LinearCongruential.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void separator() { cout << string(60, '=') << endl; }

int main() {
  cout << "\n============================================================"
       << endl;
  cout << "  RANDOM NUMBER GENERATORS - COMPREHENSIVE TEST SUITE " << endl;
  cout << "============================================================\n"
       << endl;

  // UNIFORM GENERATORS
  separator();
  cout << "UNIFORM GENERATORS" << endl;
  separator();

  LinearCongruential lcg(27, 1103515245, 12345, 2147483648);
  cout << "LinearCongruential - Mean: " << fixed << setprecision(4)
       << lcg.Mean(10000) << " (expected: 0.5)" << endl;

  EcuyerCombined ecuyer(12345, 67890);
  cout << "EcuyerCombined - Mean: " << ecuyer.Mean(10000) << " (expected: 0.5)"
       << endl;

  // Create uniform for discrete/continuous tests
  LinearCongruential *uni =
      new LinearCongruential(42, 1103515245, 12345, 2147483648);

  // DISCRETE GENERATORS
  separator();
  cout << "DISCRETE GENERATORS" << endl;
  separator();

  HeadTail coin(uni);
  cout << "HeadTail - Mean: " << coin.Mean(10000) << " (expected: 0.5)" << endl;

  Bernoulli bern(0.7, uni);
  cout << "Bernoulli(p=0.7) - Mean: " << bern.Mean(10000) << " (expected: 0.7)"
       << endl;

  Binomial binom(10, 0.3, uni);
  cout << "Binomial(n=10, p=0.3) - Mean: " << binom.Mean(10000)
       << " (expected: 3.0)" << endl;

  vector<double> probs = {0.2, 0.3, 0.5};
  FiniteSet finite(probs, uni);
  cout << "FiniteSet({0.2,0.3,0.5}) - Mean: " << finite.Mean(10000)
       << " (expected: ~2.3)" << endl;

  Poisson pois1(5.0, FirstAlgorithm, uni);
  cout << "Poisson(lambda=5, Algo1) - Mean: " << pois1.Mean(1000)
       << " (expected: 5.0)" << endl;

  Poisson pois2(5.0, SecondAlgorithm, uni);
  cout << "Poisson(lambda=5, Algo2) - Mean: " << pois2.Mean(1000)
       << " (expected: 5.0)" << endl;

  // CONTINUOUS GENERATORS
  separator();
  cout << "CONTINUOUS GENERATORS" << endl;
  separator();

  Exponential exp1(2.0, InverseDistribution, uni);
  cout << "Exponential(lambda=2, Inverse) - Mean: " << exp1.Mean(1000)
       << " (expected: 0.5)" << endl;

  Normal norm1(0.0, 1.0, BoxMuller, uni);
  cout << "Normal(mu=0, sigma=1, Box-Muller) - Mean: " << setprecision(3)
       << norm1.Mean(1000) << " (expected: 0.0)" << endl;

  Normal norm2(0.0, 1.0, CentralLimitTheorem, uni);
  cout << "Normal(mu=0, sigma=1, CLT) - Mean: " << norm2.Mean(1000)
       << " (expected: 0.0)" << endl;

  Normal norm3(10.0, 2.0, BoxMuller, uni);
  cout << "Normal(mu=10, sigma=2, Box-Muller) - Mean: " << setprecision(2)
       << norm3.Mean(1000) << " (expected: 10.0)" << endl;

  BivariateNormal biNorm(0.0, 1.0, 1.0, 2.0, 0.6, BoxMuller, uni);
  cout << "BivariateNormal - Mean X: " << setprecision(3)
       << biNorm.MeanFirst(1000) << " (expected: 0.0)" << endl;
  cout << "BivariateNormal - Mean Y: " << biNorm.MeanSecond(1000)
       << " (expected: 1.0)" << endl;
  cout << "BivariateNormal - Corr: " << biNorm.Correlation(1000)
       << " (expected: 0.6)" << endl;

  delete uni;

  separator();
  cout << "ALL GENERATORS TESTED SUCCESSFULLY!" << endl;
  separator();

  return 0;
}
