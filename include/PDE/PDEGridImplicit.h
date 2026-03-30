#pragma once

#include "PDE/PDEGrid2D.h"

class PDEGridImplicit : public PDEGrid2D {
public:
  PDEGridImplicit(double T, double MinX, double MaxX, double h0, double h1,
                  R2R1Function* a, R2R1Function* b, R2R1Function* r,
                  R2R1Function* f, R1R1Function* TopBoundaryFunction,
                  R1R1Function* BottomBoundaryFunction,
                  R1R1Function* RightBoundaryFunction);
  void FillNodes() override;
  virtual ~PDEGridImplicit();
};
