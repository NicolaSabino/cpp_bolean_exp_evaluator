#!/bin/bash

RED='01;31'
GREEN='01;32'
ACID_GREEN='01;92'
PURPLE='01;35'
YELLOW='01;33'
INI_DEMO_FILE_PATH="/tmp/demo.ini"

function _apply_color
{
    GREP_COLOR=$1 egrep --color=always '.*'
}

function _apply_yellow_color
{
    _apply_color $YELLOW
}

function _apply_green_color
{
    _apply_color $GREEN
}

function _apply_red_color
{
    _apply_color $RED
}

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
    # pkill -INT server_exe
    LD_LIBRARY_PATH=. ./server_exe 2>&1 > server.log &
    SERVER_PID=$!
}

function kill_server
{
    if [ -n "$SERVER_PID" ]; then
        # Kill the background process using the stored PID
        kill -s SIGINT $SERVER_PID
        echo "Server process with PID $SERVER_PID signaled with SIGINT."
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

function test_1
{
    run_bg_server
    local server_pid=$(pgrep -x "server_exe")

    if [[ -z "$server_pid" ]]; then
        echo "Error: Server is not running" | _apply_green_color
        return
    else
        echo "Server is running correctly with pid ${server_pid}" | _apply_green_color
    fi

    if [[ -z "$(lsof -Pi :12345 -sTCP:LISTEN -t)" ]]; then
        echo "Error: Port 12345 is not in use" | _apply_red_color
        return
    else 
        echo "Port 12345 is busy as expected" | _apply_green_color
    fi

    kill_server
    sleep 1

    if kill -0 "$server_pid" 2>/dev/null; then
        echo "Error: Server is still running with pid $server_pid" | _apply_red_color
        echo "Forcing server down with TERM" | _apply_yellow_color
        pkill -TERM server_exe
        return
    else
        echo "Server has been shut down correctly" | _apply_green_color
    fi

    if ! [[ -z "$(lsof -Pi :12345 -sTCP:LISTEN -t)" ]]; then
        echo "Error: Port 12345 is still in use" | _apply_red_color
    else
        echo "Port 12345 is no longer busy" | _apply_green_color
    fi

}

function create_ini_file
{
    touch $INI_DEMO_FILE_PATH
    echo "[section]" > /tmp/demo.ini
    echo "foo.bar = some value" >> /tmp/demo.ini
}

function test_2
{
    echo Create $INI_DEMO_FILE_PATH
    create_ini_file
    echo Server turn on
    run_bg_server
    local server_pid=$(pgrep -x "server_exe")
    res=$(./client_exe --load $INI_DEMO_FILE_PATH)
    if [ "$res" -eq 0 ]; then
        echo "File $INI_DEMO_FILE_PATH loaded successfully" | _apply_green_color
    else
        echo "Failed to load $INI_DEMO_FILE_PATH, return value $res" | _apply_red_color
    fi
    res=$(./client_exe --get section.foo.bar)
    if [ "$res" = "0 some value" ]; then
        echo "Get performed successfully with return value: $res" | _apply_green_color
    else
        echo "Failed to get data, return value: $res" | _apply_red_color
    fi
    res=$(./client_exe --get not.existing.value)
    if [ "$res" -eq 3 ]; then
        echo "Get performed successfully on non existing value with return value: $res" | _apply_green_color
    else
        echo "Failed to get not existing data, return value: $res" | _apply_red_color
    fi
    res=$(./client_exe --set section2.key1.field value)
    if [ "$res" -eq 0 ]; then
        echo "Set performed successfully with return value: $res" | _apply_green_color
    else
        echo "Failed to set data, return value: $res" | _apply_red_color
    fi

    echo Server shut down
    kill_server
}

echo "### test 1 ###"
test_1

echo "### test 2 ###"
test_2

exit 0
