cmake_minimum_required(VERSION 3.7)

project(sound-example)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

SET(PA_PRIVATE_COMPILE_DEFINITIONS PA_ENABLE_DEBUG_OUTPUT PA_LOG_API_CALLS)

SET(BUILD_TESTING OFF CACHE BOOL "")
SET(BUILD_EXAMPLES OFF CACHE BOOL "")
SET(BUILD_REGTEST OFF CACHE BOOL "")
SET(BUILD_PROGRAMS OFF CACHE BOOL "")
SET(ENABLE_CPACK OFF CACHE BOOL "")

add_subdirectory(lib/libsndfile)
add_subdirectory(lib/portaudio)

if (APPLE)
    find_library(CORE_AUDIO_LIBRARY CoreAudio)
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
    find_library(AUDIO_UNIT_LIBRARY AudioUnit)
    find_library(CARBON_LIBRARY Carbon)
    set(
            EXTRA_LIBS
            ${CORE_AUDIO_LIBRARY}
            ${AUDIO_TOOLBOX_LIBRARY}
            ${AUDIO_UNIT_LIBRARY}
            ${CARBON_LIBRARY}
    )
endif (APPLE)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(
            EXTRA_LIBS
            rt
            asound
            jack
            pthread
    )
endif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

set(
        SOURCE_FILES
        src/main.cpp
        src/util.cpp
        src/FileHandler.cpp
        src/StreamHandler.cpp
        src/AudioPlayer.cpp
)

get_target_property(
        LIBSNDFILE_INCLUDES
        sndfile
        PUBLIC_HEADER
)

include_directories(
        include
        ${LIBSNDFILE_INCLUDES}
        ${PA_PUBLIC_INCLUDES}
)

add_executable(
        sound-example
        ${SOURCE_FILES}
)

target_link_libraries(
        sound-example
        ${EXTRA_LIBS}
        sndfile
        portaudio
)

add_custom_command(
        TARGET sound-example
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/sounds" "${EXECUTABLE_OUTPUT_PATH}/sounds"
)
