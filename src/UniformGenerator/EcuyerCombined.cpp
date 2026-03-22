#include "UniformGenerator/EcuyerCombined.h"

EcuyerCombined::EcuyerCombined(double seed1, double seed2)
    : PseudoGenerator(seed1), FirstGenerator(seed1, 40014, 0, 2147483563),
      SecondGenerator(seed2, 40692, 0, 2147483399) {}

double EcuyerCombined::Generate() {

  double R1 = FirstGenerator.Generate();
  double R2 = SecondGenerator.Generate();

  double X1 = R1 * 2147483563;
  double X2 = R2 * 2147483399;

  double X = X1 - X2;
  if (X < 1) {
    X = X + 2147483562;
  }

  Seed = X;

  if (X > 0) {
    return X / 2147483563;
  } else {
    return 2147483562.0 / 2147483563.0;
  }
}
