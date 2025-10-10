cmake_minimum_required(VERSION 3.15)
project(sound-example
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "Asynchronous sound playback example with PortAudio and libsndfile"
)

# Default to local install directory unless explicitly overridden
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH
        "Install path prefix (use -DCMAKE_INSTALL_PREFIX=/custom/path for system install)"
        FORCE
    )
endif()

# Modern C++ standard setting
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use FetchContent instead of ExternalProject for better integration
include(FetchContent)

# libsndfile - now has CMake support
FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        1.2.2
    GIT_SHALLOW    TRUE
)

# Work around CMake version requirement in libsndfile 1.2.2
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# PortAudio - using GitHub mirror instead of SVN
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        v19.7.0
    GIT_SHALLOW    TRUE
)

# Configure libsndfile options
set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libsndfile portaudio)

# Platform-specific libraries
if(APPLE)
    find_library(CORE_AUDIO_LIBRARY CoreAudio REQUIRED)
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox REQUIRED)
    find_library(AUDIO_UNIT_LIBRARY AudioUnit REQUIRED)
    find_library(CARBON_LIBRARY Carbon REQUIRED)
    set(PLATFORM_LIBS
        ${CORE_AUDIO_LIBRARY}
        ${AUDIO_TOOLBOX_LIBRARY}
        ${AUDIO_UNIT_LIBRARY}
        ${CARBON_LIBRARY}
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LIBS rt asound jack pthread)
endif()

# Main executable
add_executable(sound-example
    src/main.cpp
    src/util.cpp
    src/FileHandler.cpp
    src/StreamHandler.cpp
    src/AudioPlayer.cpp
)

# Modern target-based properties
target_compile_features(sound-example PRIVATE cxx_std_14)

target_include_directories(sound-example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sound-example
    PRIVATE
        sndfile
        portaudio
        ${PLATFORM_LIBS}
)

# Copy sounds directory to build location
add_custom_command(TARGET sound-example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/sounds"
        "$<TARGET_FILE_DIR:sound-example>/sounds"
    COMMENT "Copying sound files to build directory..."
)

# Installation rules (defaults to ${CMAKE_BINARY_DIR}/install)
install(TARGETS sound-example
    RUNTIME DESTINATION bin
)

install(DIRECTORY sounds/
    DESTINATION share/sound-example/sounds
)
