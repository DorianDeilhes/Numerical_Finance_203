#include "DiscreteGenerator/Bernoulli.h"
#include "DiscreteGenerator/Binomial.h"
#include "DiscreteGenerator/FiniteSet.h"
#include "DiscreteGenerator/HeadTail.h"
#include "DiscreteGenerator/Poisson.h"
#include "UniformGenerator/LinearCongruential.h"
#include <iomanip>
#include <iostream>
#include <vector>


using namespace std;

int main() {
  // Create uniform generator for discrete tests
  LinearCongruential *uniformGen =
      new LinearCongruential(42, 1103515245, 12345, 2147483648);

  // Test HeadTail
  cout << "=== Testing HeadTail ===" << endl;
  HeadTail coin(uniformGen);
  cout << "10 flips: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)coin.Generate() << " ";
  }
  cout << endl;

  // Test Bernoulli
  cout << "\n=== Testing Bernoulli (p=0.7) ===" << endl;
  Bernoulli bern(0.7, uniformGen);
  cout << "10 trials: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)bern.Generate() << " ";
  }
  cout << endl;

  // Test Binomial
  cout << "\n=== Testing Binomial (n=10, p=0.3) ===" << endl;
  Binomial binom(10, 0.3, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)binom.Generate() << " ";
  }
  cout << endl;

  // Test FiniteSet
  cout << "\n=== Testing FiniteSet ===" << endl;
  vector<double> probas = {0.2, 0.3, 0.5};
  FiniteSet finite(probas, uniformGen);
  cout << "10 values: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)finite.Generate() << " ";
  }
  cout << endl;

  // Test Poisson Algorithm 1
  cout << "\n=== Testing Poisson (lambda=3, Algo 1) ===" << endl;
  Poisson pois1(3.0, FirstAlgorithm, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)pois1.Generate() << " ";
  }
  cout << endl;

  // Test Poisson Algorithm 2
  cout << "\n=== Testing Poisson (lambda=3, Algo 2) ===" << endl;
  Poisson pois2(3.0, SecondAlgorithm, uniformGen);
  cout << "10 samples: ";
  for (int i = 0; i < 10; i++) {
    cout << (int)pois2.Generate() << " ";
  }
  cout << endl;

  delete uniformGen;
  return 0;
}
