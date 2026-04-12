cmake_minimum_required(VERSION 3.16)

project(RandomNumberGeneration LANGUAGES CXX)

# The project originally used C++11, but we can safely use C++11 or newer
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gather all generator implementation files
file(GLOB_RECURSE RNG_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# ----------------------------------------------------------------------
# Target 1: The Interactive Simulator
# ----------------------------------------------------------------------
add_executable(interactive_sim
    interactive_sim.cpp
    ${RNG_SOURCES}
)

target_include_directories(interactive_sim PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(interactive_sim PRIVATE
    -Wall
    -Wextra
    -pedantic
)

# ----------------------------------------------------------------------
# Target 2: Comprehensive Test Suite
# ----------------------------------------------------------------------
add_executable(test_all_fixed
    tests/test_all_fixed.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_all_fixed PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_all_fixed PRIVATE
    -Wall
    -Wextra
    -pedantic
)

add_executable(test_sde_fixed
    tests/test_sde_fixed.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_sde_fixed PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_sde_fixed PRIVATE
    -Wall
    -Wextra
    -pedantic
)

add_executable(test_mc_core
    tests/test_mc_core.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_mc_core PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_mc_core PRIVATE
    -Wall
    -Wextra
    -pedantic
)

add_executable(test_basket
    tests/test_basket.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_basket PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_basket PRIVATE
    -Wall
    -Wextra
    -pedantic
)

# Enable testing via CTest (so 'make test' is natively supported)
enable_testing()
add_test(NAME comprehensive_tests COMMAND test_all_fixed)
add_test(NAME sde_tests COMMAND test_sde_fixed)
add_test(NAME mc_core_tests COMMAND test_mc_core)
add_test(NAME basket_tests COMMAND test_basket)
