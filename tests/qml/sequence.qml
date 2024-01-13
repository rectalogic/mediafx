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
import QtQuick.Effects
import QtMultimedia
import MediaFX

Item {
    MediaSequence {
        id: sequence

        anchors.fill: parent

        mediaMixers: [
            CrossFadeMixer {
            },
            WipeMixer {
                direction: WipeMixer.Direction.Down
                blindsEffect: 0.05
            },
            WipeMixer {
                direction: WipeMixer.Direction.Right
                transitionWidth: 2.0
            }
        ]

        Component.onCompleted: {
            sequence.mediaSequenceEnded.connect(MediaManager.finishEncoding);
        }

        MediaClip {
            source: Qt.resolvedUrl("../fixtures/assets/blue-320x180-30fps-3s.nut")
        }
        MediaClip {
            endTime: 3000
            source: Qt.resolvedUrl("../fixtures/assets/red-320x180-15fps-8s.nut")
        }
        MediaClip {
            source: Qt.resolvedUrl("../fixtures/assets/green-320x180-15fps-3s.nut")
        }
        MediaClip {
            endTime: 3000
            source: Qt.resolvedUrl("../fixtures/assets/red-160x120.png")
        }
        MediaClip {
            source: Qt.resolvedUrl("../fixtures/assets/yellow-320x180-15fps-3s.nut")
        }
    }
}
