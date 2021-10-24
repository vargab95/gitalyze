cmake_minimum_required(VERSION 3.13)

project(gitalyze_analysis VERSION 1.0.0 DESCRIPTION "Default analysis shared library for gitalyze.")

option(BUILD_DOC "Build documentation" ON)
option(BUILD_TEST "Build tests" ON)

find_library(M_MEM_LIB NAMES m_mem libm_mem)
find_library(M_MAP_LIB NAMES m_map libm_map)
find_library(M_LIST_LIB NAMES m_list libm_list)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_library(gitalyze_analysis SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/frequency.c)
target_link_libraries(gitalyze_analysis ${M_MAP_LIB} ${M_LIST_LIB} ${M_MEM_LIB})

set_property(TARGET gitalyze_analysis PROPERTY C_STANDARD 11)
