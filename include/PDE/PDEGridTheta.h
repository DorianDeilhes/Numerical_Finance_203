#pragma once

#include "PDE/PDEGrid2D.h"

/**
 * PDEGridTheta: theta-scheme for parabolic PDEs.
 *
 * The scheme interpolates between explicit and implicit time stepping:
 *   (v_j^k - v_j^{k-1}) / h0
 *   + theta * L^{k-1}(v^{k-1})
 *   + (1 - theta) * L^k(v^k)
 *   + f = 0
 *
 * where L is the spatial operator coming from the PDE coefficients.
 *
 * Special cases:
 *   - Theta = 0   -> explicit
 *   - Theta = 1   -> implicit
 *   - Theta = 1/2 -> Crank-Nicolson
 *
 * The class stores only the scalar theta; all other data are inherited from PDEGrid2D.
 */
class PDEGridTheta : public PDEGrid2D {
public:
  PDEGridTheta(double T, double MinX, double MaxX, double h0, double h1,
               R2R1Function* a, R2R1Function* b, R2R1Function* r,
               R2R1Function* f, R1R1Function* TopBoundaryFunction,
               R1R1Function* BottomBoundaryFunction,
               R1R1Function* RightBoundaryFunction, double Theta);
  void FillNodes() override;
  virtual ~PDEGridTheta();

private:
  double Theta;
};
