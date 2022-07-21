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
option(USE_VOICE "Enable the audio based communication" ON)
option(USE_TEXT_STRING "Enable the text string based communication" ON)

option(API_PROVIDER_GOOGLE_CLOUD_API "Enable the google cloud api speech provider" ON)

###
#  Project Info
#  ------------
#  General simple information about our project.
###
project(mrhpsspeech VERSION 1.0.1
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
                      "${SRC_DIR_PATH}/Callback/Speech/CBSpeechMethod.h"
                      "${SRC_DIR_PATH}/Callback/Speech/CBNotification.cpp"
                      "${SRC_DIR_PATH}/Callback/Speech/CBNotification.h")
                      
set(SRC_LIST_SPEECH_SOURCE)

if(USE_VOICE MATCHES ON)
    set(SRC_LIST_SPEECH_SOURCE ${SRC_LIST_SPEECH_SOURCE}
                               "${SRC_DIR_PATH}/Speech/Source/Audio/AudioBuffer.cpp"
                               "${SRC_DIR_PATH}/Speech/Source/Audio/AudioBuffer.h"
                               "${SRC_DIR_PATH}/Speech/Source/APIProvider/APIProvider.h"
                               "${SRC_DIR_PATH}/Speech/Source/Voice.cpp"
                               "${SRC_DIR_PATH}/Speech/Source/Voice.h")                             
    if(API_PROVIDER_GOOGLE_CLOUD_API MATCHES ON)
        set(SRC_LIST_SPEECH_SOURCE ${SRC_LIST_SPEECH_SOURCE}
                                   "${SRC_DIR_PATH}/Speech/Source/APIProvider/GoogleCloudAPI.cpp"
                                   "${SRC_DIR_PATH}/Speech/Source/APIProvider/GoogleCloudAPI.h")
    endif()
endif()

if(USE_TEXT_STRING MATCHES ON)
    set(SRC_LIST_SPEECH_SOURCE ${SRC_LIST_SPEECH_SOURCE}
                               "${SRC_DIR_PATH}/Speech/Source/TextString.cpp"
                               "${SRC_DIR_PATH}/Speech/Source/TextString.h")
endif()
                 
set(SRC_LIST_SPEECH "${SRC_DIR_PATH}/Speech/LocalStream.cpp"
                    "${SRC_DIR_PATH}/Speech/LocalStream.h"
                    "${SRC_DIR_PATH}/Speech/SpeechEvent.cpp"
                    "${SRC_DIR_PATH}/Speech/SpeechEvent.h"
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
                           ${SRC_LIST_SPEECH_SOURCE}
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
find_library(libmrhevdata NAMES mrhevdata REQUIRED)
find_library(libmrhpsb NAMES mrhpsb REQUIRED)
find_library(libmrhls NAMES mrhls REQUIRED)

if(USE_VOICE MATCHES ON)
    if(API_PROVIDER_GOOGLE_CLOUD_API MATCHES ON)
        find_package(google_cloud_cpp_speech REQUIRED)
        find_package(google_cloud_cpp_texttospeech REQUIRED)
    endif()
endif()

target_link_libraries(mrhpsspeech PUBLIC Threads::Threads)
target_link_libraries(mrhpsspeech PUBLIC mrhbf)
target_link_libraries(mrhpsspeech PUBLIC mrhev)
target_link_libraries(mrhpsspeech PUBLIC mrhevdata)
target_link_libraries(mrhpsspeech PUBLIC mrhpsb)
target_link_libraries(mrhpsspeech PUBLIC mrhls)

if(USE_VOICE MATCHES ON)
    if(API_PROVIDER_GOOGLE_CLOUD_API MATCHES ON)
        target_link_libraries(mrhpsspeech PUBLIC google-cloud-cpp::speech)
        target_link_libraries(mrhpsspeech PUBLIC google-cloud-cpp::texttospeech)
    endif()
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

if(USE_VOICE MATCHES ON)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_USE_VOICE=1)
    if(API_PROVIDER_GOOGLE_CLOUD_API MATCHES ON)
        target_compile_definitions(mrhpsspeech PRIVATE MRH_API_PROVIDER_GOOGLE_CLOUD_API=1)
    else()
        target_compile_definitions(mrhpsspeech PRIVATE MRH_API_PROVIDER_GOOGLE_CLOUD_API=0)
    endif()
endif()

if(USE_TEXT_STRING MATCHES ON)
    target_compile_definitions(mrhpsspeech PRIVATE MRH_SPEECH_USE_TEXT_STRING=1)
endif()

###
#  Install
#  -------
#  Application installation.
###
install(TARGETS mrhpsspeech
        DESTINATION ${BIN_INSTALL_PATH})