#include "PDE/PDEGridTheta.h"

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
  // TODO: implement the theta-scheme here using the Theta member.
  // Special cases are usually theta = 0 (explicit), 1 (implicit), and 0.5.
  PDEGrid2D::FillNodes();
}

PDEGridTheta::~PDEGridTheta() {}
