#!/bin/bash
# Complete verification script for Random Number Generators

echo "=== Step 1: Compile all generators and main test ==="
g++ -o tests/test_all_fixed tests/test_all_fixed.cpp \
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
    src/ContinuousGenerator/Normal.cpp \
    -I. -I include -std=c++11

if [ $? -eq 0 ]; then
    echo "✓ Compilation successful!"
    echo ""
    echo "=== Step 2: Run the test program ==="
    ./tests/test_all_fixed.exe
    echo ""
    echo "=== Verification complete ==="
else
    echo "✗ Compilation failed - see errors above"
fi
