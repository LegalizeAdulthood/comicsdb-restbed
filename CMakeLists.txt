cmake_minimum_required(VERSION 3.22)
project(comicsdb)

find_package(restbed REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(comicsdb)
add_subdirectory(examples)
