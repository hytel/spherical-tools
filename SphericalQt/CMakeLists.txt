#################################################################################
# The MIT License                                                               #
#                                                                               #
# Copyright © 2025-present Hillel Steinberg                                     #
#                                                                               #
# Permission is hereby granted, free of charge, to any person obtaining a copy  #
# of this software and associated documentation files (the "Software"), to deal #
# in the Software without restriction, including without limitation the rights  #
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     #
# copies of the Software, and to permit persons to whom the Software is         #
# furnished to do so, subject to the following conditions:                      #
#                                                                               #
# The above copyright notice and this permission notice shall be included in    #
# all copies or substantial portions of the Software.                           #
#                                                                               #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   #
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, #
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     #
# THE SOFTWARE.                                                                 #
#################################################################################

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Force default build type to Release if none is set
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type specified, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# Image to Cubemap C++ project
project(SphericalQt CXX)

# Set a modern C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Suppress deprecation warnings on macOS and Linux
if(APPLE)
    # AppleClang: suppress Apple OpenGL deprecation warnings
    add_compile_definitions(GL_SILENCE_DEPRECATION)
elseif(UNIX)
    # GCC/Clang on Linux: disable deprecated-declarations warnings
    add_compile_options(-Wno-deprecated-declarations)
endif()

set(CMAKE_AUTOMOC ON) # Enable automatic MOC processing for Qt
set(CMAKE_AUTORCC ON) # Enable automatic RCC processing for Qt resources
set(CMAKE_AUTOUIC ON) # Enable automatic UIC processing for Qt UI files

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/UI
)

# --- Find Qt6 ---
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGLWidgets REQUIRED)

# --- Find OpenCV ---
find_package(OpenCV REQUIRED)

# --- Find FFMPEG ---
find_library(AVCODEC_LIB avcodec)
find_library(AVFORMAT_LIB avformat)
find_library(AVUTIL_LIB avutil)
find_library(SWSCALE_LIB swscale)
find_library(AVFILTER_LIB avfilter)
find_library(SWRESAMPLE_LIB swresample)

#
# FFMPEG Libraries
#
set(FFMPEG_LIBS
    ${AVCODEC_LIB}
    ${AVFORMAT_LIB}
    ${AVUTIL_LIB}
    ${SWSCALE_LIB}
    ${AVFILTER_LIB}
    ${SWRESAMPLE_LIB}
)

# Collect C++ source files from "source" directory
file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

# Collect UI files from "UI" directory
file(GLOB_RECURSE UI_FILES
    "${CMAKE_SOURCE_DIR}/UI/*.ui"
)

# Collect QRC files from "QRC" directory
file(GLOB_RECURSE QRC_FILES
    "${CMAKE_SOURCE_DIR}/QRC/*.qrc"
)

# Create the executable
add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${UI_FILES}
    ${QRC_FILES}
)

# Binary build should be in project's folder with CMakeLists.txt
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Link against Qt
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::OpenGLWidgets
        ${OpenCV_LIBS}
        ${FFMPEG_LIBS}
)

#
# Add incude paths
#
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/UI
        ${OpenCV_INCLUDE_DIRS}
)

# Platform-specific tweaks
if(APPLE)
    # macOS bundle
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.myapp"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
elseif(WIN32)
    # Windows: set console off if you want a GUI app
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

#
# Copy luts to the Resources folder if an app was created on MacOS
if(APPLE)
    set(RESOURCES_DIR "${CMAKE_BINARY_DIR}/luts")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/luts"
            "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/luts"
    )
endif()
