cmake_minimum_required(VERSION 3.10)
project(Trees)

set(CMAKE_CXX_STANDARD 17)

set(TREES trees/abstract_tree.cpp
        trees/avl_tree.cpp
        trees/cartesian_tree.cpp
        trees/rb_tree.cpp
        trees/splay_tree.cpp)

set(TESTS tests/full_test_set.cpp)

add_library(trees ${TREES})
add_library(tests ${TESTS})

add_executable(run_tests tests/run_tests.cpp)
#add_executable(unit_tests_for_set tests/unit_tests_for_set.cpp)