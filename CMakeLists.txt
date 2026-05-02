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
add_project_executable(professor_smoke_test professor_smoke_test.cpp)
add_project_executable(professor_editable_scenario professor_editable_scenario.cpp)
add_project_executable(report_results report_results.cpp)
add_project_executable(pde_vanilla_call_demo pde_vanilla_call_demo.cpp)

# ----------------------------------------------------------------------
# Test executables
# ----------------------------------------------------------------------
add_project_executable(test_all_fixed tests/test_all_fixed.cpp)
add_project_executable(test_sde_fixed tests/test_sde_fixed.cpp)
add_project_executable(test_mc_core tests/test_mc_core.cpp)
add_project_executable(test_basket tests/test_basket.cpp)
add_project_executable(test_phase3 tests/test_phase3.cpp)
add_project_executable(test_bermudan tests/test_bermudan.cpp)
add_project_executable(test_phase6_stress tests/test_phase6_stress.cpp)

# ----------------------------------------------------------------------
# CTest registration
# ----------------------------------------------------------------------
enable_testing()

add_test(NAME comprehensive_tests COMMAND test_all_fixed)
add_test(NAME sde_tests COMMAND test_sde_fixed)
add_test(NAME mc_core_tests COMMAND test_mc_core)
add_test(NAME basket_tests COMMAND test_basket)
add_test(NAME phase3_tests COMMAND test_phase3)
add_test(NAME bermudan_tests COMMAND test_bermudan)
add_test(NAME phase6_stress_tests COMMAND test_phase6_stress)
add_test(NAME professor_smoke_test COMMAND professor_smoke_test)
add_test(NAME pde_vanilla_call_demo COMMAND pde_vanilla_call_demo)
