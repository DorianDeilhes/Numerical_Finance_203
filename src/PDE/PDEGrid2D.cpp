#include "PDE/PDEGrid2D.h"
#include <cmath>

PDEGrid2D::PDEGrid2D(double T, double MinX, double MaxX, double h0, double h1,
                     R2R1Function* a, R2R1Function* b, R2R1Function* r,
                     R2R1Function* f, R1R1Function* TopBoundaryFunction,
                     R1R1Function* BottomBoundaryFunction,
                     R1R1Function* RightBoundaryFunction) 

    : T(T),
      MinX(MinX),
      MaxX(MaxX),
      h0(h0),
      h1(h1),
      a(a),
      b(b),
      r(r),
      f(f),
      TopBoundaryFunction(TopBoundaryFunction),
      BottomBoundaryFunction(BottomBoundaryFunction),
      RightBoundaryFunction(RightBoundaryFunction) 
      { 
  NodesHeight = static_cast<size_t>(std::round(T / h0)) + 1;
  NodesWidth = static_cast<size_t>(std::round((MaxX - MinX) / h1)) + 1;
  Nodes.assign(NodesHeight, std::vector<double>(NodesWidth, 0.0));
}

PDEGrid2D::~PDEGrid2D() {
  // Destructor implementation
}

void PDEGrid2D::FillNodes() {
  // Base grid filling implementation
  FillTopAndBottomBoundary();
  FillRightBoundary();
}

double PDEGrid2D::GetValue(double time, double spot) {
  // Implementation for getting a value from the grid
}

void PDEGrid2D::FillTopAndBottomBoundary() {
  // Implementation for filling top and bottom boundaries
}

void PDEGrid2D::FillRightBoundary() {
  // Implementation for filling the right boundary
}
