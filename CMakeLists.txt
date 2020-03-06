cmake_minimum_required(VERSION 3.10)
project(Trees)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(cmake/BuildFlags.cmake)

set(TREES trees/abstract_tree.h
        trees/avl_tree.h
        trees/cartesian_tree.h
        trees/rb_tree.h
        trees/skip_list.h
        trees/splay_tree.h)

set(TESTS tests/full_test_set.h
        tests/test_framework.cpp)

add_library(trees ${TREES})
add_library(tests ${TESTS})

set_target_properties(trees PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(tests PROPERTIES LINKER_LANGUAGE CXX)

add_executable(run_tests tests/run_tests.cpp)
#add_executable(unit_tests_for_set tests/unit_tests_for_set.cpp)