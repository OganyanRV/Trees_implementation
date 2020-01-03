#!/bin/bash

# Run from build directory

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 task-name"
    exit 1
fi

TASK_PATH=../$1
CLANG_PATH=./run-clang-format.py


$CLANG_PATH -r $TASK_PATH && clang-tidy $TASK_PATH/*.cpp --
