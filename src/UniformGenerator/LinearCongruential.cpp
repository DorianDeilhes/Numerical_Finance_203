#include "UniformGenerator/LinearCongruential.h"

#include <cmath>
#include <stdexcept>

namespace {

bool IsIntegerValued(double value) {
  return std::floor(value) == value;
}

void ValidateLinearCongruentialConfig(double seed,
                                      double multiplier,
                                      double increment,
                                      double modulus) {
  if (!std::isfinite(seed) || !std::isfinite(multiplier) ||
      !std::isfinite(increment) || !std::isfinite(modulus)) {
    throw std::runtime_error("LinearCongruential requires finite parameters");
  }
  if (!IsIntegerValued(seed) || !IsIntegerValued(multiplier) ||
      !IsIntegerValued(increment) || !IsIntegerValued(modulus)) {
    throw std::runtime_error("LinearCongruential requires integer-valued parameters");
  }
  if (!(modulus > 0.0)) {
    throw std::runtime_error("LinearCongruential requires modulus > 0");
  }
  if (seed < 0.0 || seed >= modulus) {
    throw std::runtime_error("LinearCongruential requires seed in [0, modulus)");
  }
  if (multiplier < 0.0) {
    throw std::runtime_error("LinearCongruential requires multiplier >= 0");
  }
  if (increment < 0.0 || increment >= modulus) {
    throw std::runtime_error("LinearCongruential requires increment in [0, modulus)");
  }
}

}  // namespace

LinearCongruential::LinearCongruential(double seed, double multiplier,
                                       double increment, double modulus)
    : PseudoGenerator(seed), Multiplier(multiplier), Increment(increment),
      Modulus(modulus) {
  ValidateLinearCongruentialConfig(seed, multiplier, increment, modulus);
}

double LinearCongruential::Generate() {

  Seed = fmod(Multiplier * Seed + Increment, Modulus);
  if (!std::isfinite(Seed)) {
    throw std::runtime_error("LinearCongruential generated a non-finite state");
  }

  return Seed / Modulus;
}
