#########################################################################
#
#  CMAKE
#
#########################################################################

###
#  Minimum Version
#  ---------------
#  The CMake version required.
###
cmake_minimum_required(VERSION 3.1)

###
#  CMake Configuration
#  -------------------
#  Configuration settings for CMake.
#
#  NOTE:
#  These settings have to be applied before the project() setting!
###
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 14)

###
#  Project Info
#  ------------
#  General simple information about our project.
###
project(mrhclispeech VERSION 1.0
                     DESCRIPTION "MRH CLI speech binary"
                     LANGUAGES CXX)

#########################################################################
#
#  PATHS
#
#########################################################################

###
#  Binary Paths
#  ------------
#  The paths for our created binary file(s).
###
set(BIN_DIR_PATH "${CMAKE_SOURCE_DIR}/bin/")
file(MAKE_DIRECTORY ${BIN_DIR_PATH})

###
#  Build Paths
#  -----------
#  The paths for the cmake build.
###
set(BUILD_DIR_PATH "${CMAKE_SOURCE_DIR}/build/")
file(MAKE_DIRECTORY ${BUILD_DIR_PATH})

###
#  Source Paths
#  ------------
#  The paths to the source files to use.
#  Add OS specific source files in their own list.
###
set(SRC_DIR_PATH "${CMAKE_SOURCE_DIR}/src/")

set(SRC_LIST_ALL "${SRC_DIR_PATH}/Exception.h"
                 "${SRC_DIR_PATH}/Main.cpp"
                 "${SRC_DIR_PATH}/Revision.h")

###
#  Library Paths
#  -------------
#  The paths to the external libraries to use. 
#  Both the binary and include paths should be set here.
#  Add OS specific libraries in their own lists.
###
set(INCLUDE_DIR_PATH "${CMAKE_SOURCE_DIR}/include/")
set(LIB_INCLUDE_LIST "${INCLUDE_DIR_PATH}/mrhshared")

#########################################################################
#
#  TARGET
#
#########################################################################

###
#  Target
#  ------
#  The target(s) to build.
###
add_executable(mrhclispeech ${SRC_LIST_ALL})
set_target_properties(mrhclispeech
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR_PATH}
                      LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR_PATH}
                      RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR_PATH})

###
#  Required Libraries
#  ------------------
#  Libraries required by this application.
###
include_directories(${LIB_INCLUDE_LIST})

###
#  Source Definitions
#  ------------------
#  Preprocessor source definitions.
###
target_compile_definitions(mrhclispeech PRIVATE MRH_SPEECH_CLI_SOCKET_PATH="/tmp/mrhpsspeech_cli.socket")