// Copyright (c) 2023 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_clip.h"
#include "clip.h"
#include <QCoreApplication>
#include <QDebug>
#include <QList>
#include <QMediaPlayer>
#include <QMediaTimeRange>
#include <QMessageLogContext>
#include <QVideoSink>
class QUrl;

MediaClip::MediaClip()
    : Clip()
{
    // We want frames faster than realtime
    mediaPlayer.setPlaybackRate(1000);
    connect(&mediaPlayerSink, &QVideoSink::videoFrameChanged, this, &MediaClip::onVideoFrameChanged);
    mediaPlayer.setVideoSink(&mediaPlayerSink);
    connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &MediaClip::onDurationChanged);
    connect(&mediaPlayer, &QMediaPlayer::errorOccurred, this, &MediaClip::onErrorOccurred);
}

void MediaClip::onErrorOccurred(QMediaPlayer::Error error, const QString& errorString)
{
    qCritical() << "MediaClip error " << error << " " << errorString;
    QCoreApplication::exit(1);
}

void MediaClip::loadMedia(const QUrl& url)
{
    mediaPlayer.setSource(url);
}

void MediaClip::onDurationChanged(qint64 duration)
{
    setClipEnd(duration); // This will emit durationChanged as well
}

qint64 MediaClip::duration() const
{
    if (clipEnd() != -1)
        return clipEnd() - clipStart();
    else {
        return mediaPlayer.duration() - clipStart();
    }
}

void MediaClip::rateControl()
{
    auto size = bufferedFrames.size();
    if (size > MaxFrameQueueSize && mediaPlayer.isPlaying())
        mediaPlayer.pause();
    else if (size < MinFrameQueueSize && !mediaPlayer.isPlaying())
        mediaPlayer.play();
}

void MediaClip::onVideoFrameChanged(const QVideoFrame& frame)
{
    if (videoSinks().isEmpty())
        return;

    // XXX need to attempt to seek - should we do that when clipStart set, or when url set? we don't know which is set first

    auto frameTime = nextClipTime();
    if (frame.endTime() >= frameTime.end())
        return;
    if (frame.startTime() >= frameTime.start()) {
        bufferedFrames.enqueue(frame);
        rateControl();
    }
}

bool MediaClip::prepareNextVideoFrame()
{
    QVideoFrame videoFrame;
    while (!bufferedFrames.isEmpty()) {
        videoFrame = bufferedFrames.dequeue();
        if (nextClipTime().contains(videoFrame.startTime())) {
            setCurrentVideoFrame(videoFrame);
            rateControl();
            return true;
        }
    }
    rateControl();
    return false;
}

void MediaClip::setActive(bool active)
{
    Clip::setActive(active);
    if (active) {
        if (mediaPlayer.source().isEmpty()) {
            loadMedia(url());
        }
        mediaPlayer.play();
    } else {
        mediaPlayer.pause();
    }
}

void MediaClip::stop()
{
    Clip::stop();
    mediaPlayer.stop();
    mediaPlayer.setSource(QUrl());
    bufferedFrames.clear();
    bufferedFrames.squeeze();
}