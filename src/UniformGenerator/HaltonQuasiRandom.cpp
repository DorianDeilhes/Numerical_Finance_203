#include "UniformGenerator/HaltonQuasiRandom.h"

#include "UniformGenerator/Helper/FirstPrimes.h"
#include "UniformGenerator/Helper/RadicalInverse.h"
#include "UniformGenerator/Helper/ValidateQuasiRandomConfig.h"

#include <cmath>
#include <stdexcept>

namespace {

double FractionalPart(double value) {
  return value - std::floor(value);
}

}  // namespace

HaltonQuasiRandom::HaltonQuasiRandom(size_t dimension,
                                     bool useCranleyPattersonShift,
                                     double shiftSeed)
    : Dimension_(dimension),
      PointIndex_(0),
      CoordinateCursor_(0),
      UseCranleyPattersonShift_(useCranleyPattersonShift),
      Bases_(),
      ShiftByCoordinate_() {
  UniformGeneratorHelper::ValidateQuasiRandomConfig(dimension, shiftSeed);
  Bases_ = UniformGeneratorHelper::FirstPrimes(dimension);
  ShiftByCoordinate_.assign(dimension, 0.0);

  if (UseCranleyPattersonShift_) {
    // Deterministic per-coordinate shift from a scalar seed.
    // Uses a simple irrational increment to avoid repeating patterns.
    const double goldenRatioConjugate = 0.6180339887498948;
    for (size_t i = 0; i < Dimension_; ++i) {
      const double raw = shiftSeed + static_cast<double>(i + 1) * goldenRatioConjugate;
      ShiftByCoordinate_[i] = FractionalPart(raw);
    }
  }
}

double HaltonQuasiRandom::Generate() {
  if (Dimension_ == 0) {
    throw std::runtime_error("HaltonQuasiRandom requires a strictly positive dimension");
  }

  const size_t coordinate = CoordinateCursor_;
  const unsigned int base = Bases_[coordinate];
  double value = UniformGeneratorHelper::RadicalInverse(base, PointIndex_ + 1);

  if (UseCranleyPattersonShift_) {
    value += ShiftByCoordinate_[coordinate];
    value = FractionalPart(value);
  }

  CoordinateCursor_ += 1;
  if (CoordinateCursor_ == Dimension_) {
    CoordinateCursor_ = 0;
    PointIndex_ += 1;
  }

  // Keep value in open interval for inverse-CDF users.
  const double epsilon = 1e-12;
  if (value <= 0.0) {
    return epsilon;
  }
  if (value >= 1.0) {
    return 1.0 - epsilon;
  }

  return value;
}

void HaltonQuasiRandom::Reset() {
  PointIndex_ = 0;
  CoordinateCursor_ = 0;
}

void HaltonQuasiRandom::SetPointIndex(size_t pointIndex) {
  PointIndex_ = pointIndex;
  CoordinateCursor_ = 0;
}

size_t HaltonQuasiRandom::GetPointIndex() const { return PointIndex_; }

size_t HaltonQuasiRandom::GetDimension() const { return Dimension_; }
