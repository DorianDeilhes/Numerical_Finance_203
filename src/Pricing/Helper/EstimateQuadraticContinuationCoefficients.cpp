#include "Pricing/Helper/EstimateQuadraticContinuationCoefficients.h"

#include "Pricing/Helper/SolveLinearSystem3x3.h"

#include <array>
#include <stdexcept>

namespace PricingHelper {

bool EstimateQuadraticContinuationCoefficients(const std::vector<double>& states,
                                               const std::vector<double>& targets,
                                               std::array<double, 3>* coefficients) {
  if (coefficients == nullptr) {
    throw std::runtime_error("EstimateQuadraticContinuationCoefficients requires non-null coefficients");
  }
  if (states.size() != targets.size()) {
    throw std::runtime_error("EstimateQuadraticContinuationCoefficients requires matching sample sizes");
  }
  if (states.size() < 3) {
    return false;
  }

  double s0 = 0.0;
  double s1 = 0.0;
  double s2 = 0.0;
  double s3 = 0.0;
  double s4 = 0.0;

  double b0 = 0.0;
  double b1 = 0.0;
  double b2 = 0.0;

  for (size_t i = 0; i < states.size(); ++i) {
    const double x = states[i];
    const double x2 = x * x;
    const double x3 = x2 * x;
    const double x4 = x2 * x2;
    const double y = targets[i];

    s0 += 1.0;
    s1 += x;
    s2 += x2;
    s3 += x3;
    s4 += x4;

    b0 += y;
    b1 += x * y;
    b2 += x2 * y;
  }

  // Normal equations for basis [1, x, x^2]: (X^T X) beta = X^T y.
  const std::array<std::array<double, 3>, 3> normal_matrix = {{
      {{s0, s1, s2}},
      {{s1, s2, s3}},
      {{s2, s3, s4}},
  }};
  const std::array<double, 3> rhs = {{b0, b1, b2}};

  return SolveLinearSystem3x3(normal_matrix, rhs, coefficients);
}

}  // namespace PricingHelper
