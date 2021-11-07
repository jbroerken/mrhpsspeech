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
#  CMake Options
#  -------------
#  Options to use when building with CMake.
###
option(USE_METHOD_CLI "Enable the local CLI text speech method" OFF)
option(USE_METHOD_SERVER "Enable the server based text speech method" ON)
option(USE_METHOD_VOICE "Enable the internet based voice speech method" ON)

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
                      
set(SRC_LIST_SPEECH_METHOD)

if(USE_METHOD_CLI MATCHES ON OR USE_METHOD_VOICE MATCHES ON)
    set(SRC_LIST_SPEECH_METHOD ${SRC_LIST_SPEECH_METHOD}
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/IO/MessageRead.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/IO/MessageRead.h"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/IO/MessageWrite.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/IO/MessageWrite.h"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/IO/MessagePacket.h"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/MessageStream.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/MessageStream.h"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/MessageOpCode.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/MessageStream/MessageOpCode.h")
endif()

if(USE_METHOD_CLI MATCHES ON)
    set(SRC_LIST_SPEECH_METHOD ${SRC_LIST_SPEECH_METHOD}
                               "${SRC_DIR_PATH}/Speech/Method/CLI.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/CLI.h")
endif()

if(USE_METHOD_SERVER MATCHES ON)
    set(SRC_LIST_SPEECH_METHOD ${SRC_LIST_SPEECH_METHOD}
                               "${SRC_DIR_PATH}/Speech/Method/Server.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/Server.h")
endif()

if(USE_METHOD_VOICE MATCHES ON)
    set(SRC_LIST_SPEECH_METHOD ${SRC_LIST_SPEECH_METHOD}
                               "${SRC_DIR_PATH}/Speech/Method/Voice/AudioStream.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/AudioStream.h"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/AudioTrack.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/AudioTrack.h"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleSTT.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleSTT.h"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleTTS.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/Voice/GoogleTTS.h"
                               "${SRC_DIR_PATH}/Speech/Method/Voice.cpp"
                               "${SRC_DIR_PATH}/Speech/Method/Voice.h")
endif()
                 
set(SRC_LIST_SPEECH "${SRC_DIR_PATH}/Speech/SpeechMethod.cpp"
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
                           ${SRC_LIST_SPEECH_METHOD}
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

if(USE_METHOD_SERVER MATCHES ON)
    find_library(libsodium NAMES sodium REQUIRED)
endif()

if(USE_METHOD_VOICE MATCHES ON)
    find_package(google-cloud-cpp REQUIRED)
endif()

target_link_libraries(mrhpsspeech PUBLIC Threads::Threads)
target_link_libraries(mrhpsspeech PUBLIC mrhbf)
target_link_libraries(mrhpsspeech PUBLIC mrhev)
target_link_libraries(mrhpsspeech PUBLIC mrhcevs)
target_link_libraries(mrhpsspeech PUBLIC mrhpsb)
target_link_libraries(mrhpsspeech PUBLIC mrhvt)

if(USE_METHOD_SERVER MATCHES ON)
    target_link_libraries(mrhpsspeech PUBLIC sodium)
endif()

if(USE_METHOD_VOICE MATCHES ON)
    target_link_libraries(mrhpsspeech PUBLIC google-cloud-cpp::speech)
    target_link_libraries(mrhpsspeech PUBLIC google-cloud-cpp::texttospeech)
endif()

###
#  Source Definitions
#  ------------------
#  Preprocessor source definitions.
###
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_SERVICE_THREAD_COUNT=2)
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_CONFIGURATION_PATH="/usr/local/etc/mrh/mrhpservice/Speech.conf")
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_SERVICE_PRINT_INPUT=0)
target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_SERVICE_PRINT_OUTPUT=0)

if(USE_METHOD_CLI OR USE_METHOD_VOICE)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_MESSAGE_STREAM_SOCKET_DIR="/tmp/mrh/")
endif()

if(USE_METHOD_CLI MATCHES ON)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_USE_METHOD_CLI=1)
endif()

if(USE_METHOD_SERVER MATCHES ON)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_USE_METHOD_SERVER=1)
endif()

if(USE_METHOD_VOICE MATCHES ON)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_USE_METHOD_VOICE=1)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_DEVICE_CONFIG_PATH="/usr/local/etc/mrh/mrhpservice/Speech_Device.conf")
endif()

###
#  Install
#  -------
#  Application installation.
###
install(TARGETS mrhpsspeech
        DESTINATION ${BIN_INSTALL_PATH})