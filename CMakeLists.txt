cmake_minimum_required(VERSION 3.15)
project(BS1_lab5_ReaderWriter C)

set(CMAKE_C_STANDARD 99)

add_executable(BS1_lab5_ReaderWriter main.c globals.h monitor.c monitor.h reader.c reader.h writer.c writer.h)