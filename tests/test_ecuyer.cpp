#include "UniformGenerator/EcuyerCombined.h"
#include "UniformGenerator/LinearCongruential.h"
#include <iomanip>
#include <iostream>


using namespace std;

int main() {
  cout << "=== Testing EcuyerCombined ===" << endl;

  EcuyerCombined ecuyer(12345, 67890);

  cout << "First 10 values:" << endl;
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(6) << ecuyer.Generate() << endl;
  }

  cout << "\nMean of 10000 samples: " << ecuyer.Mean(10000)
       << " (expected: 0.5)" << endl;

  return 0;
}
