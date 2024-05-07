#!/bin/bash

function forward_to_server
{
    echo "Exec: $1"
    echo $1 | nc localhost 12345
    sleep 1
}

function forward_to_client
{
    echo "Exec: ./client_exe $@"
    ./client_exe $@
}

function run_bg_server
{
    LD_LIBRARY_PATH=. ./server_exe > server.log 2>&1 &
    SERVER_PID=$!
}

function kill_bg_server
{
    if [ -n "$SERVER_PID" ]; then
        # Kill the background process using the stored PID
        kill $SERVER_PID
        echo "Server process with PID $SERVER_PID killed."
    fi
}

function run_fg_server
{
    LD_LIBRARY_PATH=. ./server_exe
}

function test_server_without_client
{
    run_bg_server
    forward_to_server "LOAD foo"
    forward_to_server "LOAD test_file/config.ini"
    forward_to_server "GET Dummy"
    forward_to_server "GET Advanced.key5.subsection"
    forward_to_server "SET section.color.red roses are red"
}

function test_server_and_client
{
    run_bg_server
    sleep 1
    forward_to_client "--load foo"
    forward_to_client "--load test_file/config.ini"
    forward_to_client "--get Dummy"
    forward_to_client "--get Advanced.key5.subsection"
    forward_to_client --set section.color.red \"roses are red\"
}


test_server_and_client
read -n1 -r -p "Press enter to quit..."
kill_bg_server
exit 0
