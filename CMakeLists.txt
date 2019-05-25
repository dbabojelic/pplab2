cmake_minimum_required(VERSION 3.10)
project(2lab)

set(CMAKE_CXX_STANDARD 17)


find_package(MPI)

include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(2lab main.cpp Connect4_State.cpp Connect4_State.h Message.h)

target_link_libraries(2lab ${MPI_C_LIBRARIES})
