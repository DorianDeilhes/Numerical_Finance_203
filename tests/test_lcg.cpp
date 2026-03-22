#include "UniformGenerator/LinearCongruential.h"
#include <iomanip>
#include <iostream>


using namespace std;

int main() {
  cout << "=== Testing LinearCongruential ===" << endl;

  LinearCongruential lcg(27, 1103515245, 12345, 2147483648);

  cout << "First 10 values:" << endl;
  for (int i = 0; i < 10; i++) {
    cout << fixed << setprecision(6) << lcg.Generate() << endl;
  }

  cout << "\nMean of 10000 samples: " << lcg.Mean(10000) << " (expected: 0.5)"
       << endl;

  return 0;
}
