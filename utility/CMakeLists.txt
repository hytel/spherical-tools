cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Image to Cubemap C++ project
project(image_to_cubemap CXX)

message(STATUS ${PKG_CONFIG_PATH})

# Find package config so it can be used to find libraw, etc.
find_package(PkgConfig REQUIRED)

# Make sure it was found
if(PKG_CONFIG_FOUND)
    message(STATUS "PkgConfig found. Will use it to find LibRaw.")
else()
    message(FATAL_ERROR "PkgConfig not found. Please install it on your system.")
endif()

# Add /usr/local and potentially /opt/homebrew to CMAKE_PREFIX_PATH on macOS
if(APPLE)
    # For Intel Macs (often defaults to /usr/local)
    set(CMAKE_PREFIX_PATH "/usr/local" "${CMAKE_PREFIX_PATH}")
    # For Apple Silicon Macs (often defaults to /opt/homebrew)
    set(CMAKE_PREFIX_PATH "/opt/homebrew" "${CMAKE_PREFIX_PATH}") 
endif()

# On Linux, /usr and /usr/local are standard, and usually already searched.
# If you have custom installation paths, add them here.
# For example, if you've installed LibRaw to a non-standard directory like /home/user/my_libs
# set(CMAKE_PREFIX_PATH "/home/user/my_libs" "${CMAKE_PREFIX_PATH}")

# Ensure CMake uses pkg-config to find packages (important for cross-platform)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

# Check for the libraw module using PkgConfig
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBRAW REQUIRED libraw) # Check for "libraw" module
    if(LIBRAW_FOUND)
        message(STATUS "Found LibRaw (via pkg-config): ${LIBRAW_LIBRARIES}")
    else()
        message(FATAL_ERROR "LibRaw not found via pkg-config.")
    endif()
else()
    message(FATAL_ERROR "PkgConfig not found.")
endif()

# Make sure we have Qt6 with Core/Gui components are found
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# Add executable for Qt C++ application image_to_cubemap
qt_add_executable(image_to_cubemap image_to_cubemap.cpp)

# Link to libraw
target_link_libraries(image_to_cubemap PRIVATE ${LIBRAW_LIBRARIES})

# Link to Qt6
target_link_libraries(image_to_cubemap PRIVATE Qt6::Core Qt6::Gui)
