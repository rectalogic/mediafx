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

function onClipEnded() {
    if (internal.currentClipIndex + 1 < root.mediaClips.length) {
        var clip = root.mediaClips[internal.currentClipIndex];
        clip.clipCurrentTimeChanged.disconnect(onClipCurrentTimeChanged);
        clip.clipEnded.disconnect(onClipEnded);
        internal.currentClipIndex += 1;
        initializeClip();
    }
    internal.currentMixerIndex = (internal.currentMixerIndex + 1) % root.mediaMixers.length;
    root.mediaMixers[internal.currentMixerIndex].time = 0;
};

function onClipCurrentTimeChanged() {
    var clip = root.mediaClips[internal.currentClipIndex];
    if (internal.mixInterval.contains(clip.clipCurrentTime)) {
        internal.state = "mixer";
        root.mediaMixers[internal.currentMixerIndex].time = internal.mixInterval.duration / (clip.clipCurrentTime.start - internal.mixInterval.start);
    }
};

function initializeClip() {
    var clip = root.mediaClips[internal.currentClipIndex];
    var mixer = root.mediaMixers[internal.currentMixerIndex];
    // Last clip
    if (internal.currentClipIndex >= root.mediaClips.length - 1) {
        clip.onClipEnded.connect(root.mediaSequenceEnded)
    }
    else {
        var clampedMixDuration = Math.min(Math.min(mixer.duration, clip.clipDuration), root.mediaClips[internal.currentClipIndex + 1].clipDuration);
        internal.mixInterval = MediaManager.createInterval(clip.clipEnd - clampedMixDuration, clip.clipEnd);
        clip.clipCurrentTimeChanged.connect(onClipCurrentTimeChanged);
        clip.clipEnded.connect(onClipEnded);
    }
    internal.state = "video";
};
