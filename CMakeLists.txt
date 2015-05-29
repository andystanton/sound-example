CMAKE_MINIMUM_REQUIRED(VERSION 3.2.2)
PROJECT(sound-example)

INCLUDE(ExternalProject)

ExternalProject_Add(project_libsndfile
    GIT_REPOSITORY      https://github.com/erikd/libsndfile.git
    PREFIX              lib/libsndfile
    CONFIGURE_COMMAND   <SOURCE_DIR>/configure
    BUILD_COMMAND       make
    BUILD_IN_SOURCE     1
    INSTALL_COMMAND     echo Skipping install step for libsndfile
)

ExternalProject_Add_Step(project_libsndfile autogen
   COMMAND              <SOURCE_DIR>/autogen.sh
   DEPENDEES            update
   DEPENDERS            configure
)

ExternalProject_Get_Property(project_libsndfile BINARY_DIR)
SET(libsndfile_lib_dir "${BINARY_DIR}/src/.libs")
SET(libsndfile_inc_dir "${BINARY_DIR}/src")

ADD_LIBRARY(libsndfile STATIC IMPORTED)
SET_PROPERTY(TARGET libsndfile PROPERTY IMPORTED_LOCATION ${libsndfile_lib_dir}/libsndfile.a)

SET(LIBSNDFILE_INCLUDE_PATH "${install_dir}/src/project_libsndfile-build/src/")

ExternalProject_Add(project_portaudio
    SVN_REPOSITORY      https://subversion.assembla.com/svn/portaudio/portaudio/trunk
    SVN_TRUST_CERT      1
    PREFIX              lib/portaudio
    CONFIGURE_COMMAND   <SOURCE_DIR>/configure
    BUILD_IN_SOURCE     1
    BUILD_COMMAND       make
    INSTALL_COMMAND     echo Skipping install step for portaudio
)

ExternalProject_Get_Property(project_portaudio BINARY_DIR)
ExternalProject_Get_Property(project_portaudio SOURCE_DIR)
SET(portaudio_lib_dir "${BINARY_DIR}/lib/.libs")
SET(portaudio_inc_dir "${SOURCE_DIR}/include")

ADD_LIBRARY(portaudio STATIC IMPORTED)
SET_PROPERTY(TARGET portaudio PROPERTY IMPORTED_LOCATION "${portaudio_lib_dir}/libportaudio.a")

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

INCLUDE_DIRECTORIES(include ${libsndfile_inc_dir} ${portaudio_inc_dir})

ADD_EXECUTABLE(sound-example ${SOURCE_FILES})

SET_TARGET_PROPERTIES(sound-example
                      PROPERTIES
                      CXX_STANDARD 14)

TARGET_LINK_LIBRARIES(sound-example ${EXTRA_LIBS} libsndfile portaudio)

ADD_CUSTOM_COMMAND(TARGET sound-example PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E
                    copy_directory "${CMAKE_SOURCE_DIR}/sounds" "${CMAKE_BINARY_DIR}/sounds")
