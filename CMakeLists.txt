cmake_minimum_required(VERSION 3.0)
project(ini_handler_project)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Compile ini_handler.cpp into a shared library
add_library(ini_handler SHARED "${SRC_DIR}/ini_handler.cpp")

# Compile test.cpp into an object file
add_executable(test_exe "${SRC_DIR}/test.cpp")
target_link_libraries(test_exe ini_handler)

# Compile server.cpp into an object file
add_executable(server_exe "${SRC_DIR}/server.cpp")
target_link_libraries(server_exe ini_handler pthread)

# Compile client.cpp into an executable
add_executable(client_exe "${SRC_DIR}/client.cpp")
target_link_libraries(client_exe pthread)
