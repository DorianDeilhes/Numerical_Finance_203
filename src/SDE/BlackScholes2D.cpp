#include "SDE/BlackScholes2D.h"

BlackScholes2D::BlackScholes2D(RandomGenerator* generator, double spot1,
                               double spot2, double rate1, double rate2,
                               double vol1, double vol2, double rho)
    : RandomProcess(generator, 2), Spot1_(spot1), Spot2_(spot2), Rate1_(rate1),
      Rate2_(rate2), Vol1_(vol1), Vol2_(vol2), Rho_(rho) {}
