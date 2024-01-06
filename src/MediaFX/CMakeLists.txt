# Copyright (C) 2023 Andrew Wason
#
# This file is part of mediaFX.
#
# mediaFX is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# mediaFX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with mediaFX.
# If not, see <https://www.gnu.org/licenses/>.

find_package(PkgConfig)
pkg_search_module(ffms2 REQUIRED IMPORTED_TARGET ffms2)

option(EVENT_LOGGER "Enable event logger" OFF)

qt_add_library(mediafx STATIC
    application.cpp
    session.cpp
    media_manager.cpp
    encoder.cpp
    render_control.cpp
    media_clip.cpp
    track.cpp
    video_track.cpp
    audio_track.cpp
    media.cpp
    interval.h
)

target_include_directories(mediafx PUBLIC ./)
target_include_directories(mediafx PRIVATE ${FFMS2_INCLUDE_DIRS})
target_compile_options(mediafx PUBLIC ${FFMS2_CFLAGS})

qt_add_executable(mediafxtool
    main.cpp
)

if(EVENT_LOGGER)
    target_sources(mediafxtool PRIVATE event_logger.h)
    add_compile_definitions(EVENTLOGGER)
endif()

set_property(TARGET mediafxtool PROPERTY OUTPUT_NAME mediafx)

qt_add_qml_module(mediafx
    URI MediaFX
    QML_FILES
    qml/sequence.js
    qml/MediaSequence.qml
    qml/MultiEffectState.qml
    qml/effects/MediaMixer.qml
    qml/effects/CrossFadeMixer.qml
    qml/effects/LumaMixer.qml
)
qt_add_shaders(mediafx "shaders"
    PREFIX
    "/shaders"
    FILES
    qml/effects/crossfade.frag
    qml/effects/luma.frag
)

target_link_libraries(mediafx PUBLIC PkgConfig::ffms2 Qt6::Core Qt6::Gui Qt6::GuiPrivate Qt6::Multimedia Qt6::Qml Qt6::Quick)

if(${Qt6Quick3D_FOUND})
    target_link_libraries(mediafx PUBLIC Qt6::Quick3D)
endif()

if(${Qt6WebEngineQuick_FOUND})
    target_link_libraries(mediafx PUBLIC Qt6::WebEngineQuick)
    add_compile_definitions(WEBENGINEQUICK)
endif()

target_link_libraries(mediafxtool PRIVATE mediafx mediafxplugin)

install(TARGETS mediafxtool RUNTIME DESTINATION)
