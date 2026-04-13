#pragma once

#include "UniformGenerator.h"

#include <cstddef>
#include <vector>

// Low-discrepancy Halton sequence generator.
// This class is a UniformGenerator so it can be injected anywhere pseudo-random
// uniform generators are currently used.
class HaltonQuasiRandom : public UniformGenerator {
public:
  // Builds a Halton generator with a chosen dimension and optional random shift.
  HaltonQuasiRandom(size_t dimension,
                    bool useCranleyPattersonShift = false,
                    double shiftSeed = 0.0);

  // Returns one U(0,1)-compatible coordinate from the current Halton point.
  double Generate() override;

  // Resets point index and coordinate cursor.
  void Reset();

  // Sets the index of the next Halton point.
  void SetPointIndex(size_t pointIndex);

  // Returns the current point index.
  size_t GetPointIndex() const;

  // Returns the configured dimension.
  size_t GetDimension() const;

private:
  size_t Dimension_;
  size_t PointIndex_;
  size_t CoordinateCursor_;
  bool UseCranleyPattersonShift_;
  std::vector<unsigned int> Bases_;
  std::vector<double> ShiftByCoordinate_;
};
