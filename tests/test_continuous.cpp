#include "ContinuousGenerator/BivariateNormal.h"
#include "ContinuousGenerator/Exponential.h"
#include "ContinuousGenerator/Normal.h"
#include "UniformGenerator/LinearCongruential.h"
#include <iomanip>
#include <iostream>


using namespace std;

int main() {
  LinearCongruential *uniformGen =
      new LinearCongruential(42, 1103515245, 12345, 2147483648);

  // Test Exponential - Inverse Method
  cout << "=== Testing Exponential (lambda=2, Inverse) ===" << endl;
  Exponential exp1(2.0, InverseDistribution, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(3) << exp1.Generate() << " ";
  }
  cout << "\nMean (1000 samples): " << exp1.Mean(1000) << " (expected: 0.5)"
       << endl;

  // Test Normal - Box-Muller
  cout << "\n=== Testing Normal (mu=0, sigma=1, Box-Muller) ===" << endl;
  Normal norm1(0.0, 1.0, BoxMuller, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(3) << norm1.Generate() << " ";
  }
  cout << "\nMean (1000 samples): " << norm1.Mean(1000) << " (expected: 0.0)"
       << endl;

  // Test Normal - CLT
  cout << "\n=== Testing Normal (mu=0, sigma=1, CLT) ===" << endl;
  Normal norm2(0.0, 1.0, CentralLimitTheorem, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(3) << norm2.Generate() << " ";
  }
  cout << "\nMean (1000 samples): " << norm2.Mean(1000) << " (expected: 0.0)"
       << endl;

  // Test Normal - custom parameters
  cout << "\n=== Testing Normal (mu=10, sigma=2, Box-Muller) ===" << endl;
  Normal norm3(10.0, 2.0, BoxMuller, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(3) << norm3.Generate() << " ";
  }
  cout << "\nMean (1000 samples): " << norm3.Mean(1000) << " (expected: 10.0)"
       << endl;

    // Test Bivariate Normal
    cout << "\n=== Testing Bivariate Normal ===" << endl;
    cout << "Parameters: mu1=0, mu2=1, sigma1=1, sigma2=2, rho=0.6" << endl;
    BivariateNormal biNorm(0.0, 1.0, 1.0, 2.0, 0.6, BoxMuller, uniformGen);
    cout << "5 samples (X, Y): ";
    for (int i = 0; i < 5; i++) {
      pair<double, double> sample = biNorm.Generate();
      cout << "(" << fixed << setprecision(3) << sample.first << ", "
        << sample.second << ") ";
    }
    cout << "\nMean of X (5000 samples): " << biNorm.MeanFirst(5000)
      << " (expected: 0.0)" << endl;
    cout << "Mean of Y (5000 samples): " << biNorm.MeanSecond(5000)
      << " (expected: 1.0)" << endl;
    cout << "Correlation (5000 samples): " << biNorm.Correlation(5000)
      << " (expected: 0.6)" << endl;

  delete uniformGen;
  return 0;
}
