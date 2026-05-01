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
# Target 1b: Professor Smoke Test
# ----------------------------------------------------------------------
add_executable(professor_smoke_test
    professor_smoke_test.cpp
    ${RNG_SOURCES}
)

target_include_directories(professor_smoke_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(professor_smoke_test PRIVATE
    -Wall
    -Wextra
    -pedantic
)

# ----------------------------------------------------------------------
# Target 1c: Professor Editable Scenario
# ----------------------------------------------------------------------
add_executable(professor_editable_scenario
    professor_editable_scenario.cpp
    ${RNG_SOURCES}
)

target_include_directories(professor_editable_scenario PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(professor_editable_scenario PRIVATE
    -Wall
    -Wextra
    -pedantic
)

# ----------------------------------------------------------------------
# Target 1d: Report Results Generator
# ----------------------------------------------------------------------
add_executable(report_results
    report_results.cpp
    ${RNG_SOURCES}
)

target_include_directories(report_results PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(report_results PRIVATE
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

add_executable(test_phase3
    tests/test_phase3.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_phase3 PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_phase3 PRIVATE
    -Wall
    -Wextra
    -pedantic
)

add_executable(test_bermudan
    tests/test_bermudan.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_bermudan PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_bermudan PRIVATE
    -Wall
    -Wextra
    -pedantic
)

add_executable(test_phase6_stress
    tests/test_phase6_stress.cpp
    ${RNG_SOURCES}
)

target_include_directories(test_phase6_stress PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(test_phase6_stress PRIVATE
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
add_test(NAME phase3_tests COMMAND test_phase3)
add_test(NAME bermudan_tests COMMAND test_bermudan)
add_test(NAME phase6_stress_tests COMMAND test_phase6_stress)
add_test(NAME professor_smoke_test COMMAND professor_smoke_test)
