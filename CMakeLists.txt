cmake_minimum_required(VERSION 3.15)
project(Basic_Compiler)

set(CMAKE_CXX_STANDARD 14)

add_executable(Basic_Compiler main.cpp Lexer.h Parser.h AST.h "Instruction Generator.h")