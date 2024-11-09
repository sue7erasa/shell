cmake_minimum_required(VERSION 3.28)
project(shell C)

set(CMAKE_C_STANDARD 23)

#add_compile_options(-Wall -Wextra -Wpedantic -Werror)


add_executable(shell src/shell.c
        src/parse/parse.h
        src/parse/parse.c
        src/errors.h
        src/errors.c
        src/history.h
        src/history.c
        src/builtin_cmds.c
        src/builtin_cmds.h)
