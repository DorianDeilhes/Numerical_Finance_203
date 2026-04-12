#include "SDE/BlackScholes2D.h"
#include <stdexcept>

BlackScholes2D::BlackScholes2D(RandomGenerator* generator, double spot1,
                               double spot2, double rate1, double rate2,
                               double vol1, double vol2, double rho)
    : RandomProcess(generator, 2), Spot1_(spot1), Spot2_(spot2), Rate1_(rate1),
      Rate2_(rate2), Vol1_(vol1), Vol2_(vol2), Rho_(rho) {
  if (!(spot1 > 0.0) || !(spot2 > 0.0)) {
    throw std::runtime_error("BlackScholes2D requires strictly positive spots");
  }
  if (vol1 < 0.0 || vol2 < 0.0) {
    throw std::runtime_error("BlackScholes2D requires non-negative volatilities");
  }
  if (rho < -1.0 || rho > 1.0) {
    throw std::runtime_error("BlackScholes2D requires rho in [-1, 1]");
  }
}
