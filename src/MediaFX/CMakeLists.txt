# Copyright (C) 2023 Andrew Wason
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(PkgConfig)
pkg_search_module(libavformat REQUIRED IMPORTED_TARGET libavformat>=58.76.100)
pkg_search_module(libavcodec REQUIRED IMPORTED_TARGET libavcodec>=58.134.100)
pkg_search_module(libavfilter REQUIRED IMPORTED_TARGET libavfilter>=7.110.100)
pkg_search_module(libavutil REQUIRED IMPORTED_TARGET libavutil>=56.70.100)

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --dirty --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE MEDIAFX_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(MEDIAFX_VERSION dev)
endif()

configure_file("version.h.in" "version.h")

qt_add_library(mediafx STATIC
    application.cpp
    render_context.cpp
    render_session.cpp
    decoder.cpp
    stream.cpp
    audio_stream.cpp
    video_stream.cpp
    encoder.cpp
    output_stream.cpp
    render_control.cpp
    render_window.cpp
    media_clip.cpp
    audio_renderer.cpp
    interval.cpp
)

target_include_directories(mediafx PUBLIC ./)
target_include_directories(mediafx PRIVATE ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBAVCODEC_INCLUDE_DIRS} ${LIBAVFILTER_INCLUDE_DIRS})
target_include_directories(mediafx PUBLIC ${LIBAVUTIL_INCLUDE_DIRS})
target_compile_options(mediafx PRIVATE ${LIBAVFORMAT_CFLAGS} ${LIBAVCODEC_CFLAGS} ${LIBAVFILTER_CFLAGS} ${LIBAVUTIL_CFLAGS})

option(WITH_MSAA "Enable MSAA antialiasing." OFF)

if(WITH_MSAA)
    target_compile_definitions(mediafx PRIVATE MSAA)
endif()

qt_add_executable(mediafxtool
    main.cpp
)

# For generated version.h
target_include_directories(mediafxtool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

option(WITH_EVENT_LOGGER "Enable event logger" OFF)

if(WITH_EVENT_LOGGER)
    target_sources(mediafxtool PRIVATE event_logger.h)
    target_compile_definitions(mediafxtool PRIVATE EVENTLOGGER)
endif()

set_property(TARGET mediafxtool PROPERTY OUTPUT_NAME mediafx)

qt_add_qml_module(mediafx
    URI MediaFX
    QML_FILES
    app-encoder.qml
    app-viewer.qml
    VideoRenderer.qml
    sequence.js
    MediaSequence.qml
    MediaSequenceClip.qml
    MultiEffectState.qml
    ShaderEffectState.qml
    Transformer.qml
    DEPENDENCIES
    QtQuick
    QtQuick.Controls
    QtQuick.Effects
    QtMultimedia
    MediaFX.Viewer
    MediaFX.Transition
    MediaFX.Transition.GL
)

target_link_libraries(mediafx PUBLIC PkgConfig::libavformat PkgConfig::libavcodec PkgConfig::libavfilter PkgConfig::libavutil Qt6::Core Qt6::Gui Qt6::GuiPrivate Qt6::Multimedia Qt6::Qml Qt6::Quick)

target_link_libraries(mediafxtool PRIVATE mediafx mediafxplugin transitionplugin gltransitionplugin viewerplugin)

install(TARGETS mediafxtool RUNTIME DESTINATION)

add_subdirectory(Transition)
add_subdirectory(Viewer)
add_subdirectory(doc)