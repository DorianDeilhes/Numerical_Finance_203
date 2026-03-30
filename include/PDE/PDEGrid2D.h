#pragma once
#include "PDE/R1R1Function.h"
#include "PDE/R2R1Function.h"
#include <vector>

class PDEGrid2D {
public:
  PDEGrid2D(double T, double MinX, double MaxX, double h0, double h1,
            R2R1Function* a, R2R1Function* b, R2R1Function* r,
            R2R1Function* f, R1R1Function* TopBoundaryFunction,
            R1R1Function* BottomBoundaryFunction,
            R1R1Function* RightBoundaryFunction);
  virtual void FillNodes();
  double GetValue(double time, double spot);
  virtual ~PDEGrid2D();
protected:
  double T;
  double MinX;
  double MaxX;
  double h0;
  double h1;
  R2R1Function* a;
  R2R1Function* b;
  R2R1Function* r;
  R2R1Function* f;
  R1R1Function* TopBoundaryFunction;
  R1R1Function* BottomBoundaryFunction;
  R1R1Function* RightBoundaryFunction;
  std::vector<std::vector<double>> Nodes;
  size_t NodesHeight;
  size_t NodesWidth;

  void FillTopAndBottomBoundary();
  void FillRightBoundary();
};
