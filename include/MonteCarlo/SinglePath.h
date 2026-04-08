#pragma once

#include <vector>

class SinglePath {
public:
  SinglePath(double startTime, double endTime, size_t nbSteps);

  void InsertValue(double value);
  double GetState(double time) const;
  std::vector<double> GetAllValues() const;

private:
  std::vector<double> Values_;
  double StartTime_;
  double EndTime_;
  size_t NbSteps_;
};
