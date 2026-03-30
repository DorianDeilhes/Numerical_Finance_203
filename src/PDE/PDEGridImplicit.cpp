#include "PDE/PDEGridImplicit.h"

PDEGridImplicit::PDEGridImplicit(double T, double MinX, double MaxX, double h0,
                                 double h1, R2R1Function* a, R2R1Function* b,
                                 R2R1Function* r, R2R1Function* f,
                                 R1R1Function* TopBoundaryFunction,
                                 R1R1Function* BottomBoundaryFunction,
                                 R1R1Function* RightBoundaryFunction)
    : PDEGrid2D(T, MinX, MaxX, h0, h1, a, b, r, f, TopBoundaryFunction,
                BottomBoundaryFunction, RightBoundaryFunction) {}

void PDEGridImplicit::FillNodes() {
  // TODO: implement the implicit finite-difference scheme here.
  // This is where you will build and solve the linear system at each time step.
  PDEGrid2D::FillNodes();
}

PDEGridImplicit::~PDEGridImplicit() {}
