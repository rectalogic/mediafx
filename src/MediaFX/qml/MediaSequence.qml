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

VideoOutput {
    id: root

    default required property list<MediaClip> mediaClips
    required property list<MediaMixer> mediaMixers

    signal mediaSequenceEnded

    Item {
        id: internal

        property int currentClipIndex: 0
        property int currentMixerIndex: 0
        property interval mixInterval: MediaManager.createInterval(-1, -1)

        anchors.fill: parent

        states: [
            State {
                name: "video"

                PropertyChanges {
                    Media.clip: root.mediaClips[internal.currentClipIndex]
                    target: root
                }
                PropertyChanges {
                    target: root.mediaMixers[internal.currentMixerIndex]
                    visible: false
                }
            },
            State {
                name: "mixer"

                PropertyChanges {
                    Media.clip: root.mediaClips[internal.currentClipIndex]
                    layer.enabled: true
                    target: root
                    visible: false
                }
                PropertyChanges {
                    Media.clip: (internal.currentClipIndex + 1 >= root.mediaClips.length) ? null : root.mediaClips[internal.currentClipIndex + 1]
                    layer.enabled: true
                    target: auxVideo
                }
                ParentChange {
                    height: root.height
                    parent: root.parent
                    target: root.mediaMixers[internal.currentMixerIndex]
                    width: root.width
                    x: root.x
                    y: root.y
                }
                PropertyChanges {
                    dest: auxVideo
                    source: root
                    target: root.mediaMixers[internal.currentMixerIndex]
                    visible: true
                }
            }
        ]

        Component.onCompleted: Sequence.initializeClip()

        VideoOutput {
            id: auxVideo

            fillMode: root.fillMode
            height: root.height
            parent: root.parent
            visible: false
            width: root.width
            x: root.x
            y: root.y
        }
    }
}
