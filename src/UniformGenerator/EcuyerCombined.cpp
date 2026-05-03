#include "UniformGenerator/EcuyerCombined.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace {

const double kFirstModulus = 2147483563.0;
const double kSecondModulus = 2147483399.0;
const double kCombinedModulusMinusOne = 2147483562.0;

bool IsIntegerValued(double value) {
  return std::floor(value) == value;
}

double CheckedEcuyerSeed(double seed, double modulus, const char* seedName) {
  if (!std::isfinite(seed)) {
    throw std::runtime_error(std::string("EcuyerCombined requires finite ") + seedName);
  }
  if (!IsIntegerValued(seed)) {
    throw std::runtime_error(std::string("EcuyerCombined requires integer-valued ") + seedName);
  }
  if (!(seed > 0.0 && seed < modulus)) {
    throw std::runtime_error(std::string("EcuyerCombined requires ") + seedName +
                             " in (0, modulus)");
  }
  return seed;
}

}  // namespace

EcuyerCombined::EcuyerCombined(double seed1, double seed2)
    : PseudoGenerator(CheckedEcuyerSeed(seed1, kFirstModulus, "seed1")),
      FirstGenerator(CheckedEcuyerSeed(seed1, kFirstModulus, "seed1"), 40014, 0,
                     kFirstModulus),
      SecondGenerator(CheckedEcuyerSeed(seed2, kSecondModulus, "seed2"), 40692, 0,
                      kSecondModulus) {}

double EcuyerCombined::Generate() {

  double R1 = FirstGenerator.Generate();
  double R2 = SecondGenerator.Generate();

  double X1 = R1 * kFirstModulus;
  double X2 = R2 * kSecondModulus;

  double X = X1 - X2;
  if (X < 1) {
    X = X + kCombinedModulusMinusOne;
  }

  Seed = X;

  if (X > 0) {
    return X / kFirstModulus;
  } else {
    return kCombinedModulusMinusOne / kFirstModulus;
  }
}
