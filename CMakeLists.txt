cmake_minimum_required(VERSION 3.13)

project(gitalyze VERSION 1.0.0 DESCRIPTION "Tool to analyze changes in a git repository.")

option(BUILD_DOC "Build documentation" ON)
option(BUILD_TEST "Build tests" ON)

find_library(M_MEM_LIB NAMES m_mem libm_mem)
find_library(M_MAP_LIB NAMES m_map libm_map)
find_library(M_LIST_LIB NAMES m_list libm_list)
find_library(M_ARGS_LIB NAMES m_args libm_args)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

ExternalProject_Add(libgit2
    GIT_REPOSITORY https://github.com/libgit2/libgit2
    GIT_TAG v1.3.0
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable(gitalyze
               ${CMAKE_CURRENT_SOURCE_DIR}/main.c
               ${CMAKE_CURRENT_SOURCE_DIR}/object_tree.c
               ${CMAKE_CURRENT_SOURCE_DIR}/commit_list.c
               )
add_dependencies(gitalyze libgit2)
target_link_libraries(gitalyze git2 ${M_ARGS_LIB} ${M_MAP_LIB} ${M_LIST_LIB} ${M_MEM_LIB})

set_property(TARGET gitalyze PROPERTY C_STANDARD 11)
