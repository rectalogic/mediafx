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
import mediafx

Item {
    id: root

    required default property VideoOutput video
    required property list<MediaClip> mediaClips
    required property list<MediaMixer> mediaMixers
    property int mixerDuration: 2500 //XXX ms duration of each mixer

    Item {
        id: private

        property int currentClipIndex: 0
        property int currentMixerIndex: 0

        anchors.fill: parent

        VideoOutput {
            id: auxVideo
            parent: root.video.parent
            x: root.video.x
            y: root.video.y
            width: root.video.width
            height: root.video.height
            fillMode: root.video.fillMode
            visible: false
        }
        states: [
            State {
                name: "video"
                PropertyChanges {
                    Media.clip: root.mediaClips[currentClipIndex]
                    target: root.video
                }
            },
            State {
                name: "mixer"
                PropertyChanges {
                    Media.clip: root.mediaClips[currentClipIndex]
                    layer.enabled: true
                    visible: false
                    target: root.video
                }
                PropertyChanges {
                    Media.clip: root.mediaClips[currentClipIndex + 1]
                    layer.enabled: true
                    target: auxVideo
                }
                PropertyChanges {
                    source: root.video
                    dest: auxVideo
                    target: root.mediaMixers[currentMixerIndex]
                }
            }
        ]

        Component.onCompleted: {
            function onClipEnded() {
                if (currentClipIndex + 1 < root.mediaClips.length) {
                    currentClipIndex += 1;
                    initializeClip();
                    state = "video";
                }
                currentMixerIndex = (currentMixerIndex + 1) % root.mediaMixers.length;
                root.mediaMixers[currentMixerIndex].time = 0;
            };
            function onClipCurrentTimeChanged() {
                var clip = root.mediaClips[currentClipIndex];
                //XXX monitor for mixing time, unless beginning of first or end of last
                //XXX set state "mixer" if in interval, and drive mixer time
            };
            function initializeClip() {
                var clip = root.mediaClips[currentClipIndex];
                //XXX compute and store mixing Interval (in private property), that can be checked in onClipCurrentTimeChanged
                clip.onClipCurrentTimeChanged = onClipCurrentTimeChanged;
                clip.onClipEnded = onClipEnded;
            };
            state = "video";
            //XXX hmm, I think we want to just start monitoring first clips time and clipEnded and when we hit our interval, then state change, and when it ends, state change again - and increment currentClipIndex
            //XXX but need to hook next clips clipEnded as soon as it starts, in case it finishes early (before mixer is done)
            //XXX or we could shorten the overlap based on each clips duration
        }
    }
}