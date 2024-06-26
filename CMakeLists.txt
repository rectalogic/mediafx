# Copyright (C) 2023 Andrew Wason
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.22)

project(mediaFX VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick ShaderTools QUIET)

qt_standard_project_setup()
qt_policy(SET QTP0001 NEW)
cmake_policy(SET CMP0079 NEW)
enable_testing(true)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

if(NOT "${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "")
endif()

add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)

add_subdirectory(src/MediaFX)
add_subdirectory(tests)
