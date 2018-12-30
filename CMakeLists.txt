cmake_minimum_required(VERSION 3.13)
project(tictactoe)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(tictactoe
        cmake-build-debug/cmake_install.cmake
        cmake-build-debug/CMakeCache.txt
        cmake-build-debug/Makefile
        cmake-build-debug/tictactoe
        cmake-build-debug/tictactoe.cbp
        cmake-build-debug/tictactoe_server
        CMakeLists.txt
        LICENSE
        main.c
        message.h
        message_example.c
        server.c
        server.h)
