// Copyright (C) 2024 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Effects
import MediaFX

Item {
    MediaClip {
        id: videoClip

        source: Qt.resolvedUrl("../fixtures/assets/blue-320x180-30fps-3s-awb44100.nut")

        Component.onCompleted: {
            videoClip.clipEnded.connect(MediaManager.finishEncoding);
        }
    }
    VideoRenderer {
        id: videoRenderer

        anchors.fill: parent
        mediaClip: videoClip

        states: MultiEffectState {
            name: "filter"
            videoRenderer: videoRenderer
            // From 1-2 sec into the video, switch to greyscale
            when: (videoClip.currentFrameTime.containedBy(1000, 2000))

            MultiEffect {
                saturation: -1.0
            }
        }
    }
}
