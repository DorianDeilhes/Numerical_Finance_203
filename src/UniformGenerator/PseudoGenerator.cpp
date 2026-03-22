#include "UniformGenerator/PseudoGenerator.h"

PseudoGenerator::PseudoGenerator(double seed) : Seed(seed) {}

double PseudoGenerator::getSeed() const { return Seed; }
