cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.9)

project(taposc VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

if(WIN32)
    set(LIBLO_LIBRARY_SUFFIX lib)
else()
    set(LIBLO_LIBRARY_SUFFIX a)
endif()

add_subdirectory(third_party)
add_subdirectory(src)
