#!/bin/bash

SRC=./src

function compile_shared_library
{
    g++ -g -O0 -shared -o libini_handler.so ${SRC}/ini_handler.cpp -fPIC
}

function generate_test_lib_object
{
    g++ -g -O0 -o test.o -c ${SRC}/test.cpp
}

function generate_server_object
{
    g++ -g -O0 -o server.o -c ${SRC}/server.cpp
}

function compile_test_lib
{
    generate_test_lib_object
    g++ -g -O0 -o test_exe test.o -L. -lini_handler
}

function compile_server
{
    generate_server_object
    g++ -g -O0 -o server_exe server.o -L. -lini_handler -pthread
}

function compile_client
{
    g++ -o client_exe ${SRC}/client.cpp -pthread
}

compile_shared_library
compile_test_lib
compile_server
compile_client

