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
  // TODO: implement the explicit finite-difference update for interior nodes.
  // You will probably call the boundary-filling helpers from PDEGrid2D here too.
  PDEGrid2D::FillNodes();
}

PDEGridExplicit::~PDEGridExplicit() {}
