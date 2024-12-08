cmake_minimum_required(VERSION 3.10)
project(1_midterm)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

find_package(OpenMP REQUIRED)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

add_executable(1_midterm
        src/main.cpp
        src/renderer.cpp
)

target_link_libraries(1_midterm PRIVATE OpenMP::OpenMP_CXX)