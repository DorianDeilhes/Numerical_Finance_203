#pragma once

#include "PDE/PDEGrid2D.h"

/**
 * PDEGridExplicit: Explicit finite-difference scheme for solving parabolic PDEs.
 *
 * Method:
 *   - Backward time marching from T to 0
 *   - For each interior node (k,j), uses explicit formula:
 *
 *     V(k-1,j) = V(k,j) * (1 - 2A - B - h0*r)
 *              + V(k,j+1) * (B + A)
 *              + V(k,j-1) * A
 *              + h0*f
 *
 *   where:
 *     A = h0 * a(x_j, t_k) / h1^2
 *     B = h0 * b(x_j, t_k) / h1
 *
 * Stability: Conditional stability requires CFL-type restriction.
 *   For standard explicit schemes: 0.5 * A <= 0.5 (typically stricter in practice)
 */
class PDEGridExplicit : public PDEGrid2D {
public:
  /**
   * Constructor: Initialize explicit PDE solver with discretization and coefficients.
   * See PDEGrid2D constructor for parameter descriptions.
   */
  PDEGridExplicit(double T, double MinX, double MaxX, double h0, double h1,
                  R2R1Function* a, R2R1Function* b, R2R1Function* r,
                  R2R1Function* f, R1R1Function* TopBoundaryFunction,
                  R1R1Function* BottomBoundaryFunction,
                  R1R1Function* RightBoundaryFunction);

  /**
   * FillNodes: Compute the full solution grid using explicit discretization.
   * First fills boundary conditions, then marches backward in time with the explicit recurrence.
   */
  void FillNodes() override;

  virtual ~PDEGridExplicit();
};
