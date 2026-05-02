#include "PDE/PDEGridTheta.h"
#include "PDEFunctions/BSActualization.h"
#include "PDEFunctions/BSTrend.h"
#include "PDEFunctions/BSVariance.h"
#include "PDEFunctions/CallBottomBoundary.h"
#include "PDEFunctions/CallTerminalCondition.h"
#include "PDEFunctions/CallTopBoundary.h"
#include "PDEFunctions/NullFunction.h"

#include <cmath>
#include <iomanip>
#include <iostream>

namespace {

double StandardNormalCdf(double x) {
  return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double BlackScholesCall(double spot,
                        double strike,
                        double maturity,
                        double rate,
                        double volatility) {
  const double sqrt_maturity = std::sqrt(maturity);
  const double d1 = (std::log(spot / strike) +
                     (rate + 0.5 * volatility * volatility) * maturity) /
                    (volatility * sqrt_maturity);
  const double d2 = d1 - volatility * sqrt_maturity;

  return spot * StandardNormalCdf(d1) -
         strike * std::exp(-rate * maturity) * StandardNormalCdf(d2);
}

}  // namespace

int main() {
  const double spot = 100.0;
  const double strike = 100.0;
  const double maturity = 1.0;
  const double rate = 0.05;
  const double volatility = 0.20;

  const double min_spot = 0.0;
  const double max_spot = 300.0;
  const double time_step = 0.002;
  const double space_step = 1.0;
  const double theta = 0.5;  // Crank-Nicolson scheme.

  BSVariance variance(volatility);
  BSTrend trend(rate);
  BSActualization actualization(rate);
  NullFunction source;

  CallTopBoundary top_boundary(max_spot, strike, rate, maturity);
  CallBottomBoundary bottom_boundary(min_spot, strike, rate, maturity);
  CallTerminalCondition terminal_condition(strike);

  PDEGridTheta grid(maturity,
                    min_spot,
                    max_spot,
                    time_step,
                    space_step,
                    &variance,
                    &trend,
                    &actualization,
                    &source,
                    &top_boundary,
                    &bottom_boundary,
                    &terminal_condition,
                    theta);

  grid.FillNodes();

  const double pde_price = grid.GetValue(0.0, spot);
  const double analytic_price =
      BlackScholesCall(spot, strike, maturity, rate, volatility);
  const double absolute_error = std::fabs(pde_price - analytic_price);

  std::cout << std::setprecision(10);
  std::cout << "============================================================\n";
  std::cout << "  PDE VANILLA CALL DEMO\n";
  std::cout << "============================================================\n\n";
  std::cout << "This optional demo prices the one-asset basket special case\n";
  std::cout << "with a finite-difference Black-Scholes PDE solver.\n\n";
  std::cout << "PDE price      : " << pde_price << "\n";
  std::cout << "Analytic price : " << analytic_price << "\n";
  std::cout << "Absolute error : " << absolute_error << "\n";

  if (absolute_error > 0.25) {
    std::cerr << "PDE benchmark error is larger than expected.\n";
    return 1;
  }

  return 0;
}
