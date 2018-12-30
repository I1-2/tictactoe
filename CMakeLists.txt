cmake_minimum_required(VERSION 3.13)
project(tictactoe C)

set(CMAKE_C_STANDARD 99)

add_executable(tictactoe main.c)
add_executable(tictactoe_server server.c)