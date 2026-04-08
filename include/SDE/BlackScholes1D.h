#pragma once

#include "SDE/Brownian1D.h"

class BlackScholes1D : public RandomProcess {
public:
  BlackScholes1D(RandomGenerator* generator, double spot, double rate,
                 double vol);

protected:
  double Spot_;
  double Rate_;
  double Vol_;
};
