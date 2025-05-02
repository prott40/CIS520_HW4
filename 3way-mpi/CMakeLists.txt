cmake_minimum_required (VERSION 3.0)
project(hw4_mpi)

find_package(MPI REQUIRED)

set(CMAKE_C_FLAGS "-std=gnu11 -Wall -Wextra -Wshadow -Werror -D_XOPEN_SOURCE=500")
set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -Wextra -Wshadow -Werror -Wno-sign-compare -D_XOPEN_SOURCE=500")

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(MPI_C src/mpi.c)
target_link_libraries(MPI_C mpi)