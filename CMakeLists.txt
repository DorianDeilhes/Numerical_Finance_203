cmake_minimum_required(VERSION 3.16)

project(RandomNumberGeneration LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------
# Shared project library
# ----------------------------------------------------------------------
# All implementation files under src/ are compiled once into this library.
# Executables and tests then link to it instead of recompiling the whole project.
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(numerical_finance STATIC
    ${PROJECT_SOURCES}
)

target_include_directories(numerical_finance PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

function(apply_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()

apply_project_warnings(numerical_finance)

function(add_project_executable target_name source_file)
    add_executable(${target_name}
        ${source_file}
    )
    target_link_libraries(${target_name} PRIVATE
        numerical_finance
    )
    apply_project_warnings(${target_name})
endfunction()

# ----------------------------------------------------------------------
# User-facing executables
# ----------------------------------------------------------------------
add_project_executable(number_generation_app number_generation_app.cpp)
add_project_executable(professor_basic_test professor_basic_test.cpp)
add_project_executable(professor_editable_scenario professor_editable_scenario.cpp)
add_project_executable(report_results report_results.cpp)
add_project_executable(pde_vanilla_call_demo pde_vanilla_call_demo.cpp)

# ----------------------------------------------------------------------
# Test executables
# ----------------------------------------------------------------------
add_project_executable(test_random_generators tests/test_random_generators.cpp)
add_project_executable(test_stochastic_processes tests/test_stochastic_processes.cpp)
add_project_executable(test_monte_carlo_core tests/test_monte_carlo_core.cpp)
add_project_executable(test_european_basket tests/test_european_basket.cpp)
add_project_executable(test_variance_reduction_methods tests/test_variance_reduction_methods.cpp)
add_project_executable(test_bermudan_basket tests/test_bermudan_basket.cpp)
add_project_executable(test_dividend_pricing tests/test_dividend_pricing.cpp)
add_project_executable(test_large_basket_stress tests/test_large_basket_stress.cpp)

# ----------------------------------------------------------------------
# CTest registration
# ----------------------------------------------------------------------
enable_testing()

add_test(NAME random_generator_tests COMMAND test_random_generators)
add_test(NAME stochastic_process_tests COMMAND test_stochastic_processes)
add_test(NAME monte_carlo_core_tests COMMAND test_monte_carlo_core)
add_test(NAME european_basket_tests COMMAND test_european_basket)
add_test(NAME variance_reduction_tests COMMAND test_variance_reduction_methods)
add_test(NAME bermudan_basket_tests COMMAND test_bermudan_basket)
add_test(NAME dividend_pricing_tests COMMAND test_dividend_pricing)
add_test(NAME large_basket_stress_tests COMMAND test_large_basket_stress)
add_test(NAME professor_basic_test COMMAND professor_basic_test)
add_test(NAME pde_vanilla_call_demo COMMAND pde_vanilla_call_demo)
