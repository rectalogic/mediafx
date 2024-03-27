# Copyright (C) 2024 Andrew Wason
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_library(mixers STATIC)

# https://bugreports.qt.io/browse/QTBUG-103723
qt_add_shaders(mixers "shaders"
    PREFIX
    "/shaders"
    FILES
    crossfade.frag
    luma.frag
    pageCurl.frag
    OUTPUT_TARGETS shader_output_targets
)

foreach(shader_output_target ${shader_output_targets})
    add_dependencies(mixers ${shader_output_target})
endforeach()

qt_add_qml_module(mixers
    URI MediaFX.Mixers
    QML_FILES
    MediaMixer.qml
    MediaMixerShaderEffect.qml
    CrossFadeMixer.qml
    LumaMixer.qml
    LumaGradientMixer.qml
    PageCurlMixer.qml
    WipeMixer.qml
)