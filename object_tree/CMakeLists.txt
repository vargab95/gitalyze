cmake_minimum_required(VERSION 3.13)

project(gitalyze_object_tree VERSION 1.0.0)

option(BUILD_DOC "Build documentation" ON)
option(BUILD_TEST "Build tests" ON)

find_library(M_MEM_LIB NAMES m_mem libm_mem)
find_library(M_MAP_LIB NAMES m_map libm_map)
find_library(M_LIST_LIB NAMES m_list libm_list)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_library(gitalyze_object_tree SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/object_tree.c)
target_link_libraries(gitalyze_object_tree ${M_MAP_LIB} ${M_LIST_LIB} ${M_MEM_LIB})

set_property(TARGET gitalyze_object_tree PROPERTY C_STANDARD 11)
