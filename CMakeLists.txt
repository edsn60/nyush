cmake_minimum_required(VERSION 3.20)
project(OS_Lab2 C)

set(CMAKE_C_STANDARD 11)

add_executable(OS_Lab2 main.c nyush.c nyush.h IO_redirection.c IO_redirection.h builtin.c builtin.h command_parser.c command_parser.h external_program.c external_program.h process_manager.c process_manager.h pipe_.c pipe_.h command_check.c command_check.h structs.h structs.c)
