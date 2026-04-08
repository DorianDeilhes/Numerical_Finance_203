#include "PDE/PDEGridExplicit.h"

PDEGridExplicit::PDEGridExplicit(double T, double MinX, double MaxX, double h0,
                                 double h1, R2R1Function* a, R2R1Function* b,
                                 R2R1Function* r, R2R1Function* f,
                                 R1R1Function* TopBoundaryFunction,
                                 R1R1Function* BottomBoundaryFunction,
                                 R1R1Function* RightBoundaryFunction)
    : PDEGrid2D(T, MinX, MaxX, h0, h1, a, b, r, f, TopBoundaryFunction,
                BottomBoundaryFunction, RightBoundaryFunction) {}

void PDEGridExplicit::FillNodes() {
  PDEGrid2D::FillNodes();

  if (NodesHeight < 2 || NodesWidth < 3) {
    return;
  }

  for (size_t k = NodesHeight - 1; k > 0; --k) {
    for (size_t j = 1; j + 1 < NodesWidth; ++j) {
      const double x = MinX + static_cast<double>(j) * h1;
      const double t = static_cast<double>(k) * h0;

      const double ajk_h0_h1_sq = h0 * (*a)(x, t) / (h1 * h1);
      const double bjk_h0_h1 = h0 * (*b)(x, t) / h1;

      Nodes[k - 1][j] =
          Nodes[k][j] * (1.0 - ajk_h0_h1_sq - bjk_h0_h1 - h0 * (*r)(x, t)) +
          Nodes[k][j + 1] * (bjk_h0_h1 + 0.5 * ajk_h0_h1_sq) +
          Nodes[k][j - 1] * (0.5 * ajk_h0_h1_sq) + h0 * (*f)(x, t);
    }
  }
}

PDEGridExplicit::~PDEGridExplicit() {}
