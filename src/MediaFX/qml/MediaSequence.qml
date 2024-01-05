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
import "sequence.js" as Sequence

Item {
    id: root

    required default property VideoOutput videoOutput
    required property list<MediaClip> mediaClips
    required property list<MediaMixer> mediaMixers
    property int mixDuration: 2500 //XXX ms duration of each mixer

    signal mediaSequenceEnded()

    Item {
        id: internal

        property int currentClipIndex: 0
        property int currentMixerIndex: 0
        property interval mixInterval: MediaManager.createInterval(-1, -1)

        anchors.fill: parent

        VideoOutput {
            id: auxVideo
            parent: root.videoOutput.parent
            x: root.videoOutput.x
            y: root.videoOutput.y
            width: root.videoOutput.width
            height: root.videoOutput.height
            fillMode: root.videoOutput.fillMode
            visible: false
        }
        states: [
            State {
                name: "video"
                PropertyChanges {
                    Media.clip: root.mediaClips[currentClipIndex]
                    target: root.videoOutput
                }
            },
            State {
                name: "mixer"
                PropertyChanges {
                    Media.clip: root.mediaClips[currentClipIndex]
                    layer.enabled: true
                    visible: false
                    target: root.videoOutput
                }
                PropertyChanges {
                    Media.clip: (currentClipIndex + 1 >= root.mediaClips.length) ? null : root.mediaClips[currentClipIndex + 1]
                    layer.enabled: true
                    target: auxVideo
                }
                PropertyChanges {
                    source: root.videoOutput
                    dest: auxVideo
                    target: root.mediaMixers[currentMixerIndex]
                }
            }
        ]

        Component.onCompleted: Sequence.initializeClip()
    }
}