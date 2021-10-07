cmake_minimum_required(VERSION 3.20)
project(OS_Lab2 C)

set(CMAKE_C_STANDARD 11)

add_executable(OS_Lab2 nyush.c nyush.h builtin.c builtin.h process_manager.c process_manager.h execute_command.c execute_command.h command_parser.c command_parser.h)
