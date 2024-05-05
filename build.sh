#!/bin/bash

# Compile ini_handler.cpp into a shared library
g++ -shared -o libini_parser.so ini_handler.cpp -fPIC

# Compile test.cpp
g++ -o test.o -c test.cpp

# Link all object files and the shared library into the final executable
g++ -o executeable test.o -L. -lini_parser

# Run the test_program
LD_LIBRARY_PATH=. ./executeable
