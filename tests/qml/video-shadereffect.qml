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
import QtMultimedia
import MediaFX

Item {
    MediaClip {
        id: videoClip

        source: Qt.resolvedUrl("../fixtures/assets/blue-320x180-30fps-3s.nut")

        Component.onCompleted: {
            videoClip.clipEnded.connect(MediaManager.finishEncoding);
        }
    }
    VideoOutput {
        id: videoOutput

        Media.clip: videoClip
        anchors.fill: parent
        layer.enabled: false

        // layer.samplerName: "source"

        layer.effect: ShaderEffect {
            fragmentShader: "grayscale.frag.qsb"
        }
        states: State {
            name: "filter"
            // From 1-2 sec into the video, switch to greyscale
            when: (videoClip.clipCurrentTime.containedBy(1000, 2000))

            PropertyChanges {
                layer.enabled: true
                target: videoOutput
            }
        }
    }
}
