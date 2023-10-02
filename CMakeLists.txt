cmake_minimum_required(VERSION 3.26.4)
project(Workshop)

find_package(CudaqClassiq REQUIRED)

add_executable(tester main.cpp)
target_link_libraries(tester PRIVATE CudaqClassiq::classiq)
# Compile to GPU
# target_compile_options(tester PUBLIC --target nvidia)
