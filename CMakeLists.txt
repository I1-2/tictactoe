cmake_minimum_required(VERSION 3.13)
project(tictactoe C)

set(CMAKE_C_STANDARD 99)

add_executable(tictactoe main.c)
add_executable(tictactoe_server server.c)
add_executable(tictactoe_client client.c)
add_executable(example_tcp_server_with_select examples/tcp_server_with_select.c)
add_executable(tictactoe_bot bot.c)