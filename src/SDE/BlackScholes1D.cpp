#include "SDE/BlackScholes1D.h"

BlackScholes1D::BlackScholes1D(RandomGenerator* generator, double spot,
                               double rate, double vol)
    : RandomProcess(generator, 1), Spot_(spot), Rate_(rate), Vol_(vol) {}
