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
  FillRightBoundary();
  FillTopAndBottomBoundary();
}


double PDEGrid2D::GetValue(double time, double spot) {
  if (Nodes.empty()) {
    return 0.0;
  }

  long long timeIndex = static_cast<long long>(std::round(time / h0));
  long long spotIndex = static_cast<long long>(std::round((spot - MinX) / h1));

  if (timeIndex < 0) {
    timeIndex = 0;
  } else if (timeIndex >= static_cast<long long>(NodesHeight)) {
    timeIndex = static_cast<long long>(NodesHeight) - 1;
  }

  if (spotIndex < 0) {
    spotIndex = 0;
  } else if (spotIndex >= static_cast<long long>(NodesWidth)) {
    spotIndex = static_cast<long long>(NodesWidth) - 1;
  }

  return Nodes[static_cast<size_t>(timeIndex)][static_cast<size_t>(spotIndex)];
}

void PDEGrid2D::FillTopAndBottomBoundary() {
  for (size_t i = 0; i < NodesHeight; ++i) {
    const double time = static_cast<double>(i) * h0;
    Nodes[i][0] = BottomBoundaryFunction->operator()(time);
    Nodes[i][NodesWidth - 1] = TopBoundaryFunction->operator()(time);
  }
}

void PDEGrid2D::FillRightBoundary() {
  for (size_t j = 0; j < NodesWidth; ++j) {
    const double spot = MinX + static_cast<double>(j) * h1;
    Nodes[NodesHeight - 1][j] = RightBoundaryFunction->operator()(spot);
  }
}
