@echo off
echo Building Interactive Simulator...

if not exist build\manual mkdir build\manual

g++ -I. -I include -std=c++11 -Wall -o build\manual\interactive_sim.exe interactive_sim.cpp src/RandomGenerator.cpp src/UniformGenerator/UniformGenerator.cpp src/UniformGenerator/PseudoGenerator.cpp src/UniformGenerator/LinearCongruential.cpp src/UniformGenerator/EcuyerCombined.cpp src/DiscreteGenerator/DiscreteGenerator.cpp src/DiscreteGenerator/HeadTail.cpp src/DiscreteGenerator/Bernoulli.cpp src/DiscreteGenerator/Binomial.cpp src/DiscreteGenerator/Finiteset.cpp src/DiscreteGenerator/Poisson.cpp src/ContinuousGenerator/ContinuousGenerator.cpp src/ContinuousGenerator/BivariateNormal.cpp src/ContinuousGenerator/Exponential.cpp src/ContinuousGenerator/Normal.cpp

if %errorlevel% equ 0 (
    echo Build successful! Run it using: .\build\manual\interactive_sim.exe
) else (
    echo Build failed.
)
