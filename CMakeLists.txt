cmake_minimum_required(VERSION 3.10)
project(Trees)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if (CMAKE_BUILD_TYPE EQUAL "Release")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

include(cmake/BuildFlags.cmake)

set(TREES trees/abstract_tree.h
        trees/avl_tree.h
        trees/cartesian_tree.h
        trees/rb_tree.h
        trees/skip_list.h
        trees/splay_tree.h
        trees/stdlib_set.h
        trees/gavno.h)

set(TESTS tests/full_test_set.h
        tests/test_framework.cpp)

set(BENCHMARKS benchmarks/benchmarks.h
        benchmarks/bench_framework.cpp)

add_library(trees ${TREES})
add_library(tests ${TESTS})
add_library(bench ${BENCHMARKS})

set_target_properties(trees PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(tests PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(bench PROPERTIES LINKER_LANGUAGE CXX)

add_executable(run_all_tests tests/run_all_tests.cpp)
add_executable(run_tests_separately tests/run_tests_separately.cpp)
add_executable(run_benchmarks benchmarks/run_benchmarks.cpp)