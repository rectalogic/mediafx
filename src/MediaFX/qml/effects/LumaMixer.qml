// Copyright (C) 2024 Andrew Wason
//
// This file is part of mediaFX.
//
// mediaFX is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// mediaFX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with mediaFX.
// If not, see <https://www.gnu.org/licenses/>.

import QtQuick
import MediaFX

MediaMixer {
    id: root

    default required property Item luma
    property real transitionWidth: 1.0
    readonly property real premultipliedTransitionWidth: root.time * (transitionWidth + 1.0)

    fragmentShader: "qrc:/qml/effects/luma.frag.qsb"
    state: "default"

    states: State {
        name: "default"

        PropertyChanges {
            x: root.x
            y: root.y
            width: root.width
            height: root.height
            layer.enabled: true
            visible: false
            target: root.luma
        }
    }
}
