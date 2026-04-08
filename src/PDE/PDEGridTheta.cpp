#include "PDE/PDEGridTheta.h"
#include <algorithm>
#include <cmath>
#include <vector>

PDEGridTheta::PDEGridTheta(double T, double MinX, double MaxX, double h0,
                           double h1, R2R1Function* a, R2R1Function* b,
                           R2R1Function* r, R2R1Function* f,
                           R1R1Function* TopBoundaryFunction,
                           R1R1Function* BottomBoundaryFunction,
                           R1R1Function* RightBoundaryFunction, double Theta)
    : PDEGrid2D(T, MinX, MaxX, h0, h1, a, b, r, f, TopBoundaryFunction,
                BottomBoundaryFunction, RightBoundaryFunction),
      Theta(Theta) {}

void PDEGridTheta::FillNodes() {
  PDEGrid2D::FillNodes();

  if (NodesHeight < 2 || NodesWidth < 3) {
    return;
  }

  const double theta = std::max(0.0, std::min(1.0, Theta));
  const double explicitWeight = 1.0 - theta;
  const double implicitWeight = theta;

  for (size_t k = NodesHeight - 1; k > 0; --k) {
    const double t_curr = static_cast<double>(k) * h0;
    const double t_prev = static_cast<double>(k - 1) * h0;

    // Lecture form:
    //   (V_{k}^j - V_{k-1}^j) / h0
    //   + theta * L_{k-1}(V_{k-1})
    //   + (1 - theta) * L_k(V_k)
    //   + f = 0
    // We rewrite it as a tridiagonal linear system for the unknown row V_{k-1}.

    // Tridiagonal system for the unknown row Nodes[k - 1][j]
    std::vector<double> lower(NodesWidth, 0.0);
    std::vector<double> center(NodesWidth, 0.0);
    std::vector<double> upper(NodesWidth, 0.0);
    std::vector<double> rhs(NodesWidth, 0.0);

    // Boundary rows are fixed directly from the boundary functions.
    center[0] = 1.0;
    rhs[0] = (*BottomBoundaryFunction)(t_prev);

    center[NodesWidth - 1] = 1.0;
    rhs[NodesWidth - 1] = (*TopBoundaryFunction)(t_prev);

    for (size_t j = 1; j + 1 < NodesWidth; ++j) {
      const double x = MinX + static_cast<double>(j) * h1;

      // PDE coefficients at the known time level t_k.
      const double a_curr = (*a)(x, t_curr);
      const double b_curr = (*b)(x, t_curr);
      const double r_curr = (*r)(x, t_curr);
      const double f_curr = (*f)(x, t_curr);

      // PDE coefficients at the unknown time level t_{k-1}.
      const double a_prev = (*a)(x, t_prev);
      const double b_prev = (*b)(x, t_prev);
      const double r_prev = (*r)(x, t_prev);

      // Discrete diffusion/drift factors.
      // A = h0 * a / h1^2,  B = h0 * b / h1
      const double A_curr = h0 * a_curr / (h1 * h1);
      const double B_curr = h0 * b_curr / h1;
      const double A_prev = h0 * a_prev / (h1 * h1);
      const double B_prev = h0 * b_prev / h1;

      // Explicit part: evaluated on row k (known values Nodes[k][...]).
      // This is the contribution multiplied by (1 - theta).
      const double lower_exp = 0.5 * A_curr;
      const double center_exp = -A_curr - B_curr - h0 * r_curr;
      const double upper_exp = B_curr + 0.5 * A_curr;

      // Implicit part: evaluated on row k-1 (unknown values Nodes[k-1][...]).
      // This is the contribution multiplied by theta.
      const double lower_imp = 0.5 * A_prev;
      const double center_imp = -A_prev - B_prev - h0 * r_prev;
      const double upper_imp = B_prev + 0.5 * A_prev;

      // Final theta-combination:
      //   explicitWeight * explicit part + implicitWeight * implicit part = 0
      // moved to matrix form M * U = rhs.
      lower[j] = -implicitWeight * lower_imp;
      center[j] = 1.0 - implicitWeight * center_imp;
      upper[j] = -implicitWeight * upper_imp;

      rhs[j] = Nodes[k][j] + explicitWeight *
                                (lower_exp * Nodes[k][j - 1] +
                                 center_exp * Nodes[k][j] +
                                 upper_exp * Nodes[k][j + 1]) +
               h0 * f_curr;
    }

    // Thomas algorithm: forward sweep.
    // c'_j and d'_j are the transformed coefficients of the upper-triangular system.
    std::vector<double> c_prime(NodesWidth, 0.0);
    std::vector<double> d_prime(NodesWidth, 0.0);

    double denom0 = center[0];
    if (std::fabs(denom0) < 1e-14) {
      denom0 = 1e-14;
    }
    c_prime[0] = upper[0] / denom0;
    d_prime[0] = rhs[0] / denom0;

    for (size_t j = 1; j < NodesWidth; ++j) {
      double denom = center[j] - lower[j] * c_prime[j - 1];
      if (std::fabs(denom) < 1e-14) {
        denom = 1e-14;
      }

      if (j < NodesWidth - 1) {
        c_prime[j] = upper[j] / denom;
      }
      d_prime[j] = (rhs[j] - lower[j] * d_prime[j - 1]) / denom;
    }

    // Thomas algorithm: back substitution.
    // Recover the unknown row V_{k-1} from right to left.
    Nodes[k - 1][NodesWidth - 1] = d_prime[NodesWidth - 1];
    for (int j = static_cast<int>(NodesWidth) - 2; j >= 0; --j) {
      Nodes[k - 1][j] = d_prime[j] - c_prime[j] * Nodes[k - 1][j + 1];
    }
  }
}

PDEGridTheta::~PDEGridTheta() {}
