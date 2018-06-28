cmake_minimum_required(VERSION 3.2)


project(ProjectX C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb -Wall -Wextra")

add_subdirectory(src)
