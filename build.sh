#!/bin/bash

# Compile ini_handler.cpp into a shared library
g++ -g -O0 -shared -o libini_handler.so ini_handler.cpp -fPIC

# Compile test.cpp
g++ -g -O0 -o test.o -c test.cpp

# Link all object files and the shared library into the final executable
g++ -g -O0 -o executeable test.o -L. -lini_handler

