cmake_minimum_required(VERSION 3.10)
project(Trees)

set(CMAKE_CXX_STANDARD 17)

set(TREES trees/abstract_tree.h
        trees/avl_tree.h
        trees/cartesian_tree.h
        trees/rb_tree.h
        trees/splay_tree.h)

set(TESTS tests/full_test_set.h
        tests/test_framework.cpp)

add_library(trees ${TREES})
add_library(tests ${TESTS})

add_executable(run_tests tests/run_tests.cpp)
#add_executable(unit_tests_for_set tests/unit_tests_for_set.cpp)