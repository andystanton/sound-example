CMAKE_MINIMUM_REQUIRED(VERSION 3.6)
PROJECT(sound-example)

ADD_SUBDIRECTORY(lib/libsndfile)
ADD_SUBDIRECTORY(lib/portaudio)

IF(APPLE)
   FIND_LIBRARY(CORE_AUDIO_LIBRARY CoreAudio)
   FIND_LIBRARY(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
   FIND_LIBRARY(AUDIO_UNIT_LIBRARY AudioUnit)
   FIND_LIBRARY(CARBON_LIBRARY Carbon)
   SET(EXTRA_LIBS ${CORE_AUDIO_LIBRARY} ${AUDIO_TOOLBOX_LIBRARY} ${AUDIO_UNIT_LIBRARY} ${CARBON_LIBRARY})
ENDIF (APPLE)

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   SET(EXTRA_LIBS rt asound jack pthread)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

SET(SOURCE_FILES
    src/main.cpp
    src/util.cpp
    src/FileHandler.cpp
    src/StreamHandler.cpp
    src/AudioPlayer.cpp)

get_target_property(LIBSNDFILE_INCLUDES sndfile-static PUBLIC_HEADER)

INCLUDE_DIRECTORIES(include ${LIBSNDFILE_INCLUDES} ${PA_PUBLIC_INCLUDES})

ADD_EXECUTABLE(sound-example ${SOURCE_FILES})

SET_TARGET_PROPERTIES(sound-example
                      PROPERTIES
                      CXX_STANDARD 14)

TARGET_LINK_LIBRARIES(sound-example ${EXTRA_LIBS} sndfile-static portaudio)

ADD_CUSTOM_COMMAND(TARGET sound-example PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E
                    copy_directory "${CMAKE_SOURCE_DIR}/sounds" "${CMAKE_BINARY_DIR}/sounds")
