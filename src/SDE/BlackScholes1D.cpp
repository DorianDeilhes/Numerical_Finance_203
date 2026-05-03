#include "SDE/BlackScholes1D.h"
#include <cmath>
#include <stdexcept>

BlackScholes1D::BlackScholes1D(RandomGenerator* generator, double spot,
                               double rate, double vol)
        : RandomProcess(generator, 1), Spot_(spot), Rate_(rate), Vol_(vol) {
    if (!std::isfinite(spot) || !std::isfinite(rate) || !std::isfinite(vol)) {
        throw std::runtime_error("BlackScholes1D requires finite spot, rate and volatility");
    }
    if (!(spot > 0.0)) {
        throw std::runtime_error("BlackScholes1D requires a strictly positive spot");
    }
    if (vol < 0.0) {
        throw std::runtime_error("BlackScholes1D requires a non-negative volatility");
    }
}
