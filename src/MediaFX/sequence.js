// Copyright (C) 2024 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

function onClipEnded() {
    MediaManager.currentRenderTimeChanged.connect(nextClip);
};

function onCurrentFrameTimechanged() {
    const clip = root.mediaClips[internal.currentClipIndex];
    if (clip.currentFrameTime.start >= internal.transitionStartTime) {
        internal.state = "transition";
        root.mediaTransitions[internal.currentTransitionIndex].time = (clip.currentFrameTime.start - internal.transitionStartTime) / (clip.endTime - internal.transitionStartTime);
    }
};

function nextClip() {
    MediaManager.currentRenderTimeChanged.disconnect(nextClip);
    if (internal.currentClipIndex + 1 < root.mediaClips.length) {
        const clip = root.mediaClips[internal.currentClipIndex];
        clip.currentFrameTimeChanged.disconnect(onCurrentFrameTimechanged);
        clip.clipEnded.disconnect(onClipEnded);
        internal.currentClipIndex += 1;
        initializeClip();
    }
    internal.currentTransitionIndex = (internal.currentTransitionIndex + 1) % root.mediaTransitions.length;
    root.mediaTransitions[internal.currentTransitionIndex].time = 0;
};

function initializeClip() {
    const clip = root.mediaClips[internal.currentClipIndex];
    // Last clip
    if (internal.currentClipIndex >= root.mediaClips.length - 1) {
        clip.onClipEnded.connect(root.mediaSequenceEnded)
    }
    else {
        const transition = root.mediaTransitions[internal.currentTransitionIndex];
        const clampedTransitionDuration = Math.min(Math.min(transition.duration, clip.duration), root.mediaClips[internal.currentClipIndex + 1].duration);
        internal.transitionStartTime = clip.endTime - clampedTransitionDuration;
        clip.currentFrameTimeChanged.connect(onCurrentFrameTimechanged);
        clip.clipEnded.connect(onClipEnded);
    }
    internal.state = "video";
};
