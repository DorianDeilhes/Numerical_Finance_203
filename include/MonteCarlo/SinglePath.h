#pragma once

#include <vector>

// Stores one simulated time-discretized path of a scalar state variable.
class SinglePath {
public:
  // Creates a path container over [startTime, endTime] with nbSteps steps.
  SinglePath(double startTime, double endTime, size_t nbSteps);

  // Appends one state value at the next time index.
  void InsertValue(double value);

  // Returns the stored state associated with the closest discretized time.
  double GetState(double time) const;

  // Returns all stored values in insertion order.
  std::vector<double> GetAllValues() const;

private:
  std::vector<double> Values_;
  double StartTime_;
  double EndTime_;
  size_t NbSteps_;
};
