#pragma once

#include "PDE/PDEGrid2D.h"

/**
 * PDEGridImplicit: Implicit finite-difference scheme for solving parabolic PDEs.
 *
 * Method:
 *   - Backward time marching from T to 0
 *   - For each time step, solves a tridiagonal linear system:
 *
 *     α_j * V(k-1,j-1) + β_j * V(k-1,j) + γ_j * V(k-1,j+1) = rhs_j
 *
 *   where coefficients depend on A = h0*a(x_j,t_k)/h1² and B = h0*b(x_j,t_k)/h1
 *
 * Advantages over explicit:
 *   - Unconditionally stable (no CFL restriction)
 *   - Can use larger time steps
 *
 * Disadvantage:
 *   - Requires solving linear system at each time step (higher computational cost per step)
 */
class PDEGridImplicit : public PDEGrid2D {
public:
  /**
   * Constructor: Initialize implicit PDE solver with discretization and coefficients.
   * See PDEGrid2D constructor for parameter descriptions.
   */
  PDEGridImplicit(double T, double MinX, double MaxX, double h0, double h1,
                  R2R1Function* a, R2R1Function* b, R2R1Function* r,
                  R2R1Function* f, R1R1Function* TopBoundaryFunction,
                  R1R1Function* BottomBoundaryFunction,
                  R1R1Function* RightBoundaryFunction);

  /**
   * FillNodes: Compute the full solution grid using implicit discretization.
   * First fills boundary conditions, then marches backward in time solving tridiagonal systems.
   */
  void FillNodes() override;

  virtual ~PDEGridImplicit();
};
