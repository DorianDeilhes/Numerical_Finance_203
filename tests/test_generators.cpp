#include "DiscreteGenerator/Bernoulli.h"
#include "RandomGenerator.h"
#include "UniformGenerator/LinearCongruential.h"
#include "UniformGenerator/PseudoGenerator.h"
#include <iomanip>
#include <iostream>


using namespace std;

int main() {
  cout << "=== Random Number Generator Test ===" << endl << endl;

  // Test 1: LinearCongruential Generator
  cout << "Test 1: LinearCongruential Generator (Uniform [0,1])" << endl;
  cout << "Parameters: seed=27, a=1103515245, c=12345, m=2147483648" << endl;

  LinearCongruential lcg(27, 1103515245, 12345, 2147483648);

  cout << "First 10 random numbers:" << endl;
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(6) << lcg.Generate() << endl;
  }

  cout << "\nEmpirical mean from 100,000 samples: " << lcg.Mean(100000)
       << " (expected: 0.5)" << endl;

  cout << "\n" << string(50, '-') << "\n" << endl;

  // Test 2: Bernoulli Generator
  cout << "Test 2: Bernoulli Generator (p = 0.3)" << endl;

  LinearCongruential *uniformGen =
      new LinearCongruential(42, 1103515245, 12345, 2147483648);
  Bernoulli bernoulli(0.3, uniformGen);

  cout << "First 20 outcomes (0 or 1):" << endl;
  for (int i = 0; i < 20; i++) {
    cout << bernoulli.Generate() << " ";
    if ((i + 1) % 10 == 0)
      cout << endl;
  }

  cout << "\nEmpirical mean from 10,000 samples: " << bernoulli.Mean(10000)
       << " (expected: 0.3)" << endl;

  delete uniformGen;

  cout << "\n=== All Tests Complete ===" << endl;

  return 0;
}
