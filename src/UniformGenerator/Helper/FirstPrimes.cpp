#include "UniformGenerator/Helper/FirstPrimes.h"

#include <stdexcept>

namespace UniformGeneratorHelper {

std::vector<unsigned int> FirstPrimes(size_t count) {
  std::vector<unsigned int> primes;
  primes.reserve(count);

  unsigned int candidate = 2U;
  while (primes.size() < count) {
    bool isPrime = true;
    for (size_t i = 0; i < primes.size(); ++i) {
      const unsigned int p = primes[i];
      if (p * p > candidate) {
        break;
      }
      if (candidate % p == 0U) {
        isPrime = false;
        break;
      }
    }

    if (isPrime) {
      primes.push_back(candidate);
    }
    candidate += 1U;
  }

  return primes;
}

}  // namespace UniformGeneratorHelper
