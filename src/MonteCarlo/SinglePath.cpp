#include "MonteCarlo/SinglePath.h"
#include <algorithm>
#include <cmath>

SinglePath::SinglePath(double startTime, double endTime, size_t nbSteps)
    : StartTime_(startTime), EndTime_(endTime), NbSteps_(nbSteps) {
  Values_.reserve(nbSteps + 1);
}

void SinglePath::InsertValue(double value) {
  Values_.push_back(value);
}

double SinglePath::GetState(double time) const {
  if (Values_.empty() || NbSteps_ == 0 || EndTime_ <= StartTime_) {
    return 0.0;
  }

  const double position = (time - StartTime_) / (EndTime_ - StartTime_);
  double scaledIndex = position * static_cast<double>(NbSteps_);
  if (scaledIndex < 0.0) {
    scaledIndex = 0.0;
  }
  if (scaledIndex > static_cast<double>(Values_.size() - 1)) {
    scaledIndex = static_cast<double>(Values_.size() - 1);
  }

  return Values_[static_cast<size_t>(std::round(scaledIndex))];
}

std::vector<double> SinglePath::GetAllValues() const {
  return Values_;
}
