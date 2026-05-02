#include "PDE/PDEGridImplicit.h"
#include <vector>
#include <cmath>

PDEGridImplicit::PDEGridImplicit(double T, double MinX, double MaxX, double h0,
                                 double h1, R2R1Function* a, R2R1Function* b,
                                 R2R1Function* r, R2R1Function* f,
                                 R1R1Function* TopBoundaryFunction,
                                 R1R1Function* BottomBoundaryFunction,
                                 R1R1Function* RightBoundaryFunction)
    : PDEGrid2D(T, MinX, MaxX, h0, h1, a, b, r, f, TopBoundaryFunction,
                BottomBoundaryFunction, RightBoundaryFunction) {}

void PDEGridImplicit::FillNodes() {
  // First fill boundary conditions
  PDEGrid2D::FillNodes();

  if (NodesHeight < 2 || NodesWidth < 3) {
    return;
  }

  // Backward time marching: k = NodesHeight-1 down to 1
  for (size_t k = NodesHeight - 1; k > 0; --k) {
    // Build and solve tridiagonal system for time step k -> k-1
    // System: α * V[j-1] + β * V[j] + γ * V[j+1] = rhs[j]

    std::vector<double> lower(NodesWidth);   // sub-diagonal
    std::vector<double> center(NodesWidth);  // main diagonal
    std::vector<double> upper(NodesWidth);   // super-diagonal
    std::vector<double> rhs(NodesWidth);     // right-hand side

    const double t = static_cast<double>(k) * h0;

    center[0] = 1.0;
    rhs[0] = (*BottomBoundaryFunction)(static_cast<double>(k - 1) * h0);
    center[NodesWidth - 1] = 1.0;
    rhs[NodesWidth - 1] = (*TopBoundaryFunction)(static_cast<double>(k - 1) * h0);

    // Build tridiagonal system for interior nodes
    for (size_t j = 1; j + 1 < NodesWidth; ++j) {
      const double x = MinX + static_cast<double>(j) * h1;

      const double ajk_h0_h1_sq = h0 * (*a)(x, t) / (h1 * h1);
      const double bjk_h0_h1 = h0 * (*b)(x, t) / h1;
      const double rjk = (*r)(x, t);
      const double fjk = (*f)(x, t);

      // Coefficients for the implicit scheme:
      // -A * V[j-1] + (1 + 2A + B + h0*r) * V[j]
      // - (B + A) * V[j+1] = V[k,j] + h0*f
      lower[j] = -ajk_h0_h1_sq;
      center[j] = 1.0 + 2.0 * ajk_h0_h1_sq + bjk_h0_h1 + h0 * rjk;
      upper[j] = -(bjk_h0_h1 + ajk_h0_h1_sq);
      rhs[j] = Nodes[k][j] + h0 * fjk;
    }

    // Solve tridiagonal system using Thomas algorithm (forward elimination)
    std::vector<double> c_prime(NodesWidth);
    std::vector<double> d_prime(NodesWidth);

    c_prime[0] = 0.0;
    d_prime[0] = rhs[0];

    for (size_t j = 1; j < NodesWidth; ++j) {
      double denom = center[j] - lower[j] * c_prime[j - 1];
      if (std::fabs(denom) < 1e-14) {
        denom = 1e-14; // Avoid division by zero
      }
      if (j < NodesWidth - 1) {
        c_prime[j] = upper[j] / denom;
      }
      d_prime[j] = (rhs[j] - lower[j] * d_prime[j - 1]) / denom;
    }

    // Back substitution
    Nodes[k - 1][NodesWidth - 1] = d_prime[NodesWidth - 1];
    for (int j = static_cast<int>(NodesWidth) - 2; j >= 0; --j) {
      Nodes[k - 1][j] = d_prime[j] - c_prime[j] * Nodes[k - 1][j + 1];
    }
  }
}

PDEGridImplicit::~PDEGridImplicit() {}
