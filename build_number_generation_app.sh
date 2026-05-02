#!/bin/bash

echo "Building Number Generation App..."

mkdir -p build/manual
mkdir -p build/manual/obj

# Compile objects into build/manual/obj so the project root stays clean.
g++ -I. -I include -std=c++11 -Wall -c number_generation_app.cpp -o build/manual/obj/number_generation_app.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/RandomGenerator.cpp -o build/manual/obj/RandomGenerator.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/UniformGenerator/UniformGenerator.cpp -o build/manual/obj/UniformGenerator.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/UniformGenerator/PseudoGenerator.cpp -o build/manual/obj/PseudoGenerator.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/UniformGenerator/LinearCongruential.cpp -o build/manual/obj/LinearCongruential.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/UniformGenerator/EcuyerCombined.cpp -o build/manual/obj/EcuyerCombined.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/DiscreteGenerator/DiscreteGenerator.cpp -o build/manual/obj/DiscreteGenerator.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/DiscreteGenerator/HeadTail.cpp -o build/manual/obj/HeadTail.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/DiscreteGenerator/Bernoulli.cpp -o build/manual/obj/Bernoulli.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/DiscreteGenerator/Binomial.cpp -o build/manual/obj/Binomial.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/DiscreteGenerator/Finiteset.cpp -o build/manual/obj/Finiteset.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/DiscreteGenerator/Poisson.cpp -o build/manual/obj/Poisson.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/ContinuousGenerator/ContinuousGenerator.cpp -o build/manual/obj/ContinuousGenerator.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/ContinuousGenerator/BivariateNormal.cpp -o build/manual/obj/BivariateNormal.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/ContinuousGenerator/Exponential.cpp -o build/manual/obj/Exponential.o || exit 1
g++ -I. -I include -std=c++11 -Wall -c src/ContinuousGenerator/Normal.cpp -o build/manual/obj/Normal.o || exit 1

g++ -o build/manual/number_generation_app.exe \
    build/manual/obj/number_generation_app.o \
    build/manual/obj/RandomGenerator.o \
    build/manual/obj/UniformGenerator.o \
    build/manual/obj/PseudoGenerator.o \
    build/manual/obj/LinearCongruential.o \
    build/manual/obj/EcuyerCombined.o \
    build/manual/obj/DiscreteGenerator.o \
    build/manual/obj/HeadTail.o \
    build/manual/obj/Bernoulli.o \
    build/manual/obj/Binomial.o \
    build/manual/obj/Finiteset.o \
    build/manual/obj/Poisson.o \
    build/manual/obj/ContinuousGenerator.o \
    build/manual/obj/BivariateNormal.o \
    build/manual/obj/Exponential.o \
    build/manual/obj/Normal.o

if [ $? -eq 0 ]; then
    echo "Build successful! Run it using: ./build/manual/number_generation_app.exe"
else
    echo "Build failed."
    exit 1
fi
