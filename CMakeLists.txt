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
project(mrhpsspeech VERSION 1.0.0
                    DESCRIPTION "MRH speech platform service executable"
                    LANGUAGES CXX)

#########################################################################
#
#  PATHS
#
#########################################################################

###
#  Install Paths
#  -------------
#  The paths for our created binary file(s).
###
set(BIN_INSTALL_PATH "/usr/local/bin/")

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
			 
set(SRC_LIST_CALLBACK "${SRC_DIR_PATH}/Callback/Service/CBAvail.cpp"
                      "${SRC_DIR_PATH}/Callback/Service/CBAvail.h"
                      "${SRC_DIR_PATH}/Callback/Service/CBCustomCommand.cpp"
                      "${SRC_DIR_PATH}/Callback/Service/CBCustomCommand.h"
                      "${SRC_DIR_PATH}/Callback/Speech/CBSayString.cpp"
                      "${SRC_DIR_PATH}/Callback/Speech/CBSayString.h"
                      "${SRC_DIR_PATH}/Callback/Speech/CBSpeechMethod.cpp"
                      "${SRC_DIR_PATH}/Callback/Speech/CBSpeechMethod.h")
                 
set(SRC_LIST_SPEECH "${SRC_DIR_PATH}/Speech/Method/Voice/VoiceAudio.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/VoiceAudio.h"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/RateConverter.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/RateConverter.h"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/PADevice.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/PADevice.h"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/PocketSphinx.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/PocketSphinx.h"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleSTT.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleSTT.h"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleTTS.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleTTS.h"
                    "${SRC_DIR_PATH}/Speech/Method/CLI.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/CLI.h"
                    "${SRC_DIR_PATH}/Speech/Method/Voice.cpp"
                    "${SRC_DIR_PATH}/Speech/Method/Voice.h"
                    "${SRC_DIR_PATH}/Speech/SpeechMethod.cpp"
                    "${SRC_DIR_PATH}/Speech/SpeechMethod.h"
                    "${SRC_DIR_PATH}/Speech/OutputStorage.cpp"
                    "${SRC_DIR_PATH}/Speech/OutputStorage.h"
                    "${SRC_DIR_PATH}/Speech/Speech.cpp"
                    "${SRC_DIR_PATH}/Speech/Speech.h")

set(SRC_LIST_SERVICE "${SRC_DIR_PATH}/Configuration.cpp"
                     "${SRC_DIR_PATH}/Configuration.h"
                     "${SRC_DIR_PATH}/Exception.h"
                     "${SRC_DIR_PATH}/Revision.h"
                     "${SRC_DIR_PATH}/Main.cpp")

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
add_executable(mrhpsspeech ${SRC_LIST_CALLBACK}
                           ${SRC_LIST_SPEECH}
                           ${SRC_LIST_SERVICE})

###
#  Required Libraries
#  ------------------
#  Libraries required by this platform service.
###
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)
find_library(libmrhbf NAMES mrhbf REQUIRED)
find_library(libmrhev NAMES mrhev REQUIRED)
find_library(libmrhcevs NAMES mrhcevs REQUIRED)
find_library(libmrhpsb NAMES mrhpsb REQUIRED)
find_library(libmrhvt NAMES mrhvt REQUIRED)
find_library(libsodium NAMES sodium REQUIRED)
find_library(libportaudio NAMES portaudio REQUIRED)
find_library(libsamplerate NAMES samplerate REQUIRED)
find_library(libsphinxbase NAMES sphinxbase REQUIRED)
find_library(libpocketsphinx NAMES pocketsphinx REQUIRED)

if(APPLE)
    include_directories("/usr/local/include/sphinxbase")
    include_directories("/usr/local/include/pocketsphinx")
endif()

target_link_libraries(mrhpsspeech PUBLIC Threads::Threads)
target_link_libraries(mrhpsspeech PUBLIC mrhbf)
target_link_libraries(mrhpsspeech PUBLIC mrhev)
target_link_libraries(mrhpsspeech PUBLIC mrhcevs)
target_link_libraries(mrhpsspeech PUBLIC mrhpsb)
target_link_libraries(mrhpsspeech PUBLIC mrhvt)
target_link_libraries(mrhpsspeech PUBLIC sodium)
target_link_libraries(mrhpsspeech PUBLIC portaudio)
target_link_libraries(mrhpsspeech PUBLIC samplerate)
target_link_libraries(mrhpsspeech PUBLIC sphinxbase)
target_link_libraries(mrhpsspeech PUBLIC pocketsphinx)

###
#  Source Definitions
#  ------------------
#  Preprocessor source definitions.
###
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_SERVICE_THREAD_COUNT=2)
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_CONFIGURATION_PATH="/usr/local/etc/mrh/mrhpservice/Speech.conf")
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_CLI_ENABLED=1)
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_CLI_SOCKET_PATH="/tmp/mrhpsspeech_cli.socket")
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_SERVICE_PRINT_INPUT=1)
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_SERVICE_PRINT_OUTPUT=1)

###
#  Install
#  -------
#  Application installation.
###
install(TARGETS mrhpsspeech
        DESTINATION ${BIN_INSTALL_PATH})