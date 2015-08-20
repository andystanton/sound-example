CMAKE_MINIMUM_REQUIRED(VERSION 3.2.2)
PROJECT(sound-example)

INCLUDE(ExternalProject)

ExternalProject_Add(project_libsndfile
    GIT_REPOSITORY      https://github.com/erikd/libsndfile.git
    PREFIX              lib/libsndfile
    CONFIGURE_COMMAND   <SOURCE_DIR>/configure && <SOURCE_DIR>/autogen.sh
    BUILD_COMMAND       make
    INSTALL_COMMAND     echo Skipping install step for libsndfile
    UPDATE_COMMAND      ""
)

ExternalProject_Get_Property(project_libsndfile BINARY_DIR)
SET(libsndfile_lib_dir "${BINARY_DIR}/src/.libs")
SET(libsndfile_inc_dir "${BINARY_DIR}/src")

ADD_LIBRARY(libsndfile STATIC IMPORTED)
SET_PROPERTY(TARGET libsndfile PROPERTY IMPORTED_LOCATION ${libsndfile_lib_dir}/libsndfile.a)

SET(LIBSNDFILE_INCLUDE_PATH "${install_dir}/src/project_libsndfile-build/src/")

IF(APPLE)
    FIND_LIBRARY(OPENAL_LIBRARY OpenAL)
    FIND_PATH(OPENAL_INCLUDE_DIR al.h)
ELSE()
    SET(LIBTYPE STATIC)
    SET(OPENAL_INCLUDE_DIR lib/openal/include/AL)
    SET(OPENAL_LIBRARY openal)
    ADD_SUBDIRECTORY(lib/openal)
ENDIF()

SET(SOURCE_FILES
    src/main.cpp
    src/util.cpp
    src/FileHandler.cpp
    src/StreamHandler.cpp
    src/AudioPlayer.cpp)

INCLUDE_DIRECTORIES(include ${libsndfile_inc_dir} ${OPENAL_INCLUDE_DIR})

ADD_EXECUTABLE(sound-example ${SOURCE_FILES})

SET_TARGET_PROPERTIES(sound-example
                      PROPERTIES
                      CXX_STANDARD 14)

TARGET_LINK_LIBRARIES(sound-example ${OPENAL_LIBRARY} libsndfile)

ADD_CUSTOM_COMMAND(TARGET sound-example PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E
                    copy_directory "${CMAKE_SOURCE_DIR}/sounds" "${CMAKE_BINARY_DIR}/sounds")
