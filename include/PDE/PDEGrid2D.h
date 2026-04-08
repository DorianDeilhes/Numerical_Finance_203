#pragma once
#include "PDE/R1R1Function.h"
#include "PDE/R2R1Function.h"
#include <vector>

/**
 * PDEGrid2D: Base class for solving parabolic PDEs on a 2D grid using finite differences.
 *
 * Solves:
 *   ∂V/∂t = a(x,t) * ∂²V/∂x² + b(x,t) * ∂V/∂x - r(x,t) * V + f(x,t)
 *
 * Grid structure:
 *   - Time: t_k = k * h0, k = 0, 1, ..., NodesHeight-1
 *   - Space: x_j = MinX + j * h1, j = 0, 1, ..., NodesWidth-1
 *   - Nodes[k][j] ≈ V(t_k, x_j)
 *
 * Boundary conditions:
 *   - Terminal: V(T, x) at t = T (set by RightBoundaryFunction)
 *   - Spatial: V(t, MinX) and V(t, MaxX) for all t (BottomBoundaryFunction, TopBoundaryFunction)
 */
class PDEGrid2D {
public:
  /**
   * Constructor: Initialize PDE grid with discretization parameters and function coefficients.
   *
   * @param T Maturity time
   * @param MinX Minimum underlying value
   * @param MaxX Maximum underlying value
   * @param h0 Time step size
   * @param h1 Space step size
   * @param a R2R1Function: diffusion coefficient a(x,t)
   * @param b R2R1Function: drift coefficient b(x,t)
   * @param r R2R1Function: discount rate coefficient r(x,t)
   * @param f R2R1Function: source term f(x,t)
   * @param RightBoundaryFunction R1R1Function: terminal condition V(T,x)
   * @param BottomBoundaryFunction R1R1Function: boundary at x_min for all t
   * @param TopBoundaryFunction R1R1Function: boundary at x_max for all t
   */
  PDEGrid2D(double T, double MinX, double MaxX, double h0, double h1,
            R2R1Function* a, R2R1Function* b, R2R1Function* r,
            R2R1Function* f, R1R1Function* TopBoundaryFunction,
            R1R1Function* BottomBoundaryFunction,
            R1R1Function* RightBoundaryFunction);

  /**
   * FillNodes: Fill boundary conditions on the grid.
   * Derived classes override to implement the full PDE discretization scheme.
   */
  virtual void FillNodes();

  /**
   * GetValue: Query solution at an arbitrary (time, spot) point.
   * Returns nearest grid node value with clamping to grid bounds.
   *
   * @param time Query time (0 to T)
   * @param spot Query spot price (MinX to MaxX)
   * @return Approximate solution value V(time, spot)
   */
  double GetValue(double time, double spot);

  virtual ~PDEGrid2D();
protected:
  // PDE parameters
  double T;            ///< Maturity time
  double MinX;         ///< Minimum spot value
  double MaxX;         ///< Maximum spot value
  double h0;           ///< Time step (Δt)
  double h1;           ///< Space step (Δx)

  // PDE coefficients (function pointers)
  R2R1Function* a;     ///< Diffusion term: a(x,t)
  R2R1Function* b;     ///< Drift term: b(x,t)
  R2R1Function* r;     ///< Discount term: r(x,t)
  R2R1Function* f;     ///< Source term: f(x,t)

  // Boundary condition functions
  R1R1Function* TopBoundaryFunction;     ///< V(t, MaxX)
  R1R1Function* BottomBoundaryFunction;  ///< V(t, MinX)
  R1R1Function* RightBoundaryFunction;   ///< V(T, x) [terminal/payoff]

  // Grid storage: Nodes[time_index][space_index]
  std::vector<std::vector<double>> Nodes;
  size_t NodesHeight;  ///< Number of time steps (including t=0)
  size_t NodesWidth;   ///< Number of space steps (including boundaries)

  /**
   * FillTopAndBottomBoundary: Set spatial boundary values for all time steps.
   * Sets V(t, MinX) and V(t, MaxX) using BottomBoundaryFunction and TopBoundaryFunction.
   */
  void FillTopAndBottomBoundary();

  /**
   * FillRightBoundary: Set terminal (maturity) condition.
   * Sets V(T, x) using RightBoundaryFunction at the last time row.
   */
  void FillRightBoundary();
};
