#!/bin/bash

echo "Building Interactive Simulator..."

g++ -I. -I include -std=c++11 -Wall -o interactive_sim.exe interactive_sim.cpp \
    src/RandomGenerator.cpp \
    src/UniformGenerator/UniformGenerator.cpp \
    src/UniformGenerator/PseudoGenerator.cpp \
    src/UniformGenerator/LinearCongruential.cpp \
    src/UniformGenerator/EcuyerCombined.cpp \
    src/DiscreteGenerator/DiscreteGenerator.cpp \
    src/DiscreteGenerator/HeadTail.cpp \
    src/DiscreteGenerator/Bernoulli.cpp \
    src/DiscreteGenerator/Binomial.cpp \
    src/DiscreteGenerator/FiniteSet.cpp \
    src/DiscreteGenerator/Poisson.cpp \
    src/ContinuousGenerator/ContinuousGenerator.cpp \
    src/ContinuousGenerator/BivariateNormal.cpp \
    src/ContinuousGenerator/Exponential.cpp \
    src/ContinuousGenerator/Normal.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Run it using: ./interactive_sim.exe"
else
    echo "Build failed."
    exit 1
fi
