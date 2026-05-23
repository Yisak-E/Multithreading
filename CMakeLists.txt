cmake_minimum_required(VERSION 3.26)
project(Multithreading C)

set(CMAKE_C_STANDARD 11)

find_package(OpenMP REQUIRED)

add_executable(Multithreading
        main.c
        fold1_program.c
        fold2_program.c
        fold3_program.c
)

target_link_libraries(Multithreading PUBLIC OpenMP::OpenMP_C)
