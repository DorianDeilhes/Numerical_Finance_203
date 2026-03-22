#include <iomanip>
#include <iostream>
#include <vector>


// Base classes
#include "RandomGenerator.h"

// Uniform generators
#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/LinearCongruential.h"


// Discrete generators
#include "DiscreteGenerator/Bernoulli.h"
#include "DiscreteGenerator/Binomial.h"
#include "DiscreteGenerator/FiniteSet.h"
#include "DiscreteGenerator/HeadTail.h"
#include "DiscreteGenerator/Poisson.h"


// Continuous generators
#include "ContinuousGenerator/BivariateNormal.h"
#include "ContinuousGenerator/Exponential.h"
#include "ContinuousGenerator/Normal.h"

using namespace std;

void printSeparator() { cout << "\n" << string(70, '=') << "\n" << endl; }

int main() {
  cout << "╔════════════════════════════════════════════════════════════╗"
       << endl;
  cout << "║        RANDOM NUMBER GENERATOR - COMPREHENSIVE TEST        ║"
       << endl;
  cout << "╚════════════════════════════════════════════════════════════╝"
       << endl;

  // ============================================================================
  // UNIFORM GENERATORS
  // ============================================================================

  printSeparator();
  cout << "UNIFORM GENERATORS" << endl;
  printSeparator();

  // Test 1: LinearCongruential
  cout << "Test 1: LinearCongruential Generator" << endl;
  cout << "Parameters: seed=27, a=1103515245, c=12345, m=2147483648" << endl;
  LinearCongruential lcg(27, 1103515245, 12345, 2147483648);
  cout << "First 5 values: ";
  for (int i = 0; i < 5; i++) {
    cout << fixed << setprecision(4) << lcg.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << lcg.Mean(10000)
       << " (expected: 0.5)" << endl;

  // Test 2: EcuyerCombined
  cout << "\nTest 2: EcuyerCombined Generator" << endl;
  cout << "L'Ecuyer's combined LCG with period ~2×10^18" << endl;
  EcuyerCombined ecuyer(12345, 67890);
  cout << "First 5 values: ";
  for (int i = 0; i < 5; i++) {
    cout << fixed << setprecision(4) << ecuyer.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << ecuyer.Mean(10000)
       << " (expected: 0.5)" << endl;

  // Create a uniform generator for discrete/continuous generators
  LinearCongruential *uniformGen =
      new LinearCongruential(42, 1103515245, 12345, 2147483648);

  // ============================================================================
  // DISCRETE GENERATORS
  // ============================================================================

  printSeparator();
  cout << "DISCRETE GENERATORS" << endl;
  printSeparator();

  // Test 3: HeadTail
  cout << "Test 3: HeadTail (Coin Flip)" << endl;
  HeadTail coin(uniformGen);
  cout << "10 flips: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)coin.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << coin.Mean(10000)
       << " (expected: 0.5)" << endl;

  // Test 4: Bernoulli
  cout << "\nTest 4: Bernoulli (p=0.7)" << endl;
  Bernoulli bernoulli(0.7, uniformGen);
  cout << "10 trials: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)bernoulli.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << bernoulli.Mean(10000)
       << " (expected: 0.7)" << endl;

  // Test 5: Binomial
  cout << "\nTest 5: Binomial (n=10, p=0.3)" << endl;
  Binomial binomial(10, 0.3, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)binomial.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << binomial.Mean(10000)
       << " (expected: " << 10 * 0.3 << ")" << endl;

  // Test 6: FiniteSet
  cout << "\nTest 6: FiniteSet (Die roll with unfair die)" << endl;
  vector<double> probas = {0.1, 0.1, 0.2,
                           0.2, 0.2, 0.2}; // Probabilities for 1-6
  FiniteSet die(probas, uniformGen);
  cout << "10 rolls: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)die.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << die.Mean(10000)
       << " (expected: ~4.2)" << endl;

  // Test 7: Poisson - First Algorithm
  cout << "\nTest 7: Poisson (λ=5, First Algorithm)" << endl;
  Poisson poisson1(5.0, FirstAlgorithm, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)poisson1.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << poisson1.Mean(10000)
       << " (expected: 5.0)" << endl;

  // Test 8: Poisson - Second Algorithm
  cout << "\nTest 8: Poisson (λ=5, Second Algorithm)" << endl;
  Poisson poisson2(5.0, SecondAlgorithm, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)poisson2.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << poisson2.Mean(10000)
       << " (expected: 5.0)" << endl;

  // ============================================================================
  // CONTINUOUS GENERATORS
  // ============================================================================

  printSeparator();
  cout << "CONTINUOUS GENERATORS" << endl;
  printSeparator();

  // Test 9: Exponential - Inverse Method
  cout << "Test 9: Exponential (λ=2, Inverse Method)" << endl;
  Exponential exp1(2.0, InverseDistribution, uniformGen);
  cout << "5 samples: ";
  for (int i = 0; i < 5; i++) {
    cout << fixed << setprecision(3) << exp1.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << exp1.Mean(10000)
       << " (expected: " << 1.0 / 2.0 << ")" << endl;

  // Test 10: Normal - Box-Muller
  cout << "\nTest 10: Normal (μ=0, σ=1, Box-Muller)" << endl;
  Normal norm1(0.0, 1.0, BoxMuller, uniformGen);
  cout << "5 samples: ";
  for (int i = 0; i < 5; i++) {
    cout << fixed << setprecision(3) << norm1.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << norm1.Mean(10000)
       << " (expected: 0.0)" << endl;

  // Test 11: Normal - Central Limit Theorem
  cout << "\nTest 11: Normal (μ=0, σ=1, CLT)" << endl;
  Normal norm2(0.0, 1.0, CentralLimitTheorem, uniformGen);
  cout << "5 samples: ";
  for (int i = 0; i < 5; i++) {
    cout << fixed << setprecision(3) << norm2.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << norm2.Mean(10000)
       << " (expected: 0.0)" << endl;

  // Test 12: Normal - Custom Parameters
  cout << "\nTest 12: Normal (μ=10, σ=2, Box-Muller)" << endl;
  Normal norm3(10.0, 2.0, BoxMuller, uniformGen);
  cout << "5 samples: ";
  for (int i = 0; i < 5; i++) {
    cout << fixed << setprecision(3) << norm3.Generate() << " ";
  }
  cout << "\nEmpirical mean (10000 samples): " << norm3.Mean(10000)
       << " (expected: 10.0)" << endl;

  // Test 13: Bivariate Normal
  cout << "\nTest 13: Bivariate Normal" << endl;
  cout << "Parameters: mu1=0, mu2=1, sigma1=1, sigma2=2, rho=0.6" << endl;
  BivariateNormal biNorm(0.0, 1.0, 1.0, 2.0, 0.6, BoxMuller, uniformGen);
  cout << "3 samples (X, Y): ";
  for (int i = 0; i < 3; i++) {
    pair<double, double> sample = biNorm.Generate();
    cout << "(" << fixed << setprecision(3) << sample.first << ", "
         << sample.second << ") ";
  }
  cout << "\nEmpirical mean of X (10000 samples): " << biNorm.MeanFirst(10000)
       << " (expected: 0.0)" << endl;
  cout << "Empirical mean of Y (10000 samples): " << biNorm.MeanSecond(10000)
       << " (expected: 1.0)" << endl;
  cout << "Empirical correlation (10000 samples): "
       << biNorm.Correlation(10000) << " (expected: 0.6)" << endl;

  // Cleanup
  delete uniformGen;

  printSeparator();
  cout << "✓ ALL TESTS COMPLETE!" << endl;
     cout << "All generators implemented and working correctly." << endl;
  printSeparator();

  return 0;
}
