/*
 * Copyright (C) 2023 Andrew Wason
 *
 * This file is part of mediaFX.
 *
 * mediaFX is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * mediaFX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with mediaFX.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "media_clip.h"
#include "audio_track.h"
#include "error_info.h"
#include "interval.h"
#include "media_manager.h"
#include "session.h"
#include "video_track.h"
#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QQmlInfo>
#include <QString>
#include <QUrl>
#include <algorithm>
#include <chrono>
#include <ffms.h>

MediaClip::MediaClip(QObject* parent)
    : QObject(parent)
    , m_clipStart(-1)
    , m_clipEnd(-1)
    , m_currentFrameTime(-1, -1)
    , m_videoTrack(new VideoTrack(this))
    , m_audioTrack(new AudioTrack(this))
{
}

MediaClip::~MediaClip() = default;

void MediaClip::setSource(const QUrl& url)
{
    if (url.isEmpty()) {
        qmlWarning(this) << "MediaClip source is a write-once property and cannot be changed";
        return;
    }
    if (!url.isLocalFile()) {
        qmlWarning(this) << "MediaClip source requires a local file Url";
        return;
    }
    m_source = url;
}

void MediaClip::setClipStart(qint64 ms)
{
    if (m_clipStart != -1) {
        qmlWarning(this) << "MediaClip clipStart is a write-once property and cannot be changed";
        return;
    }
    m_clipStart = ms;
    emit clipStartChanged();
    emit clipDurationChanged();
}

void MediaClip::setClipEnd(qint64 ms)
{
    if (m_clipEnd != -1) {
        qmlWarning(this) << "MediaClip clipEnd is a write-once property and cannot be changed";
        return;
    }
    m_clipEnd = ms;
    emit clipEndChanged();
    emit clipDurationChanged();
}

void MediaClip::render()
{
    if (!isActive())
        return;

    if (m_currentFrameTime.start() >= m_clipEnd) {
        if (m_audioTrack)
            m_audioTrack->stop();
        if (m_videoTrack)
            m_videoTrack->stop();
        emit clipEnded();
        return;
    }

    emit clipCurrentTimeChanged();

    if (m_audioTrack)
        m_audioTrack->render(m_currentFrameTime);
    if (m_videoTrack)
        m_videoTrack->render(m_currentFrameTime);

    m_currentFrameTime = m_currentFrameTime.nextInterval(MediaManager::singletonInstance()->session()->frameDuration());
}

void MediaClip::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        auto mediaFX = MediaManager::singletonInstance();
        if (active) {
            mediaFX->registerClip(this);
        } else {
            mediaFX->unregisterClip(this);
        }
    }
}

void MediaClip::updateActive()
{
    setActive((m_videoTrack && m_videoTrack->isActive()) || (m_audioTrack && m_audioTrack->isActive()));
}

void MediaClip::loadMedia()
{
    if (!source().isValid()) {
        qmlWarning(this) << "MediaClip requires source Url";
        emit MediaManager::singletonInstance()->session()->exitApp(1);
        return;
    }
    ErrorInfo errorInfo;
    QString sourceFile = source().toLocalFile();
    QByteArray sourceFileUtf8 = sourceFile.toUtf8();
    FFMS_Indexer* indexer = FFMS_CreateIndexer(sourceFileUtf8.data(), &errorInfo);
    if (!indexer) {
        qmlWarning(this) << "MediaClip FFMS_CreateIndexer failed:" << errorInfo;
        emit MediaManager::singletonInstance()->session()->exitApp(1);
        return;
    }

    FFMS_Index* index = FFMS_DoIndexing2(indexer, FFMS_IEH_ABORT, &errorInfo);
    if (!index) {
        qmlWarning(this) << "MediaClip FFMS_DoIndexing2 failed:" << errorInfo;
        emit MediaManager::singletonInstance()->session()->exitApp(1);
        return;
    }

    if (!m_videoTrack->initialize(index, sourceFileUtf8.data(), errorInfo)) {
        m_videoTrack.reset(nullptr);
        qmlWarning(this) << "VideoTrack initialize failed:" << errorInfo;
        errorInfo.reset();
    }

    if (m_audioTrack->initialize(index, sourceFileUtf8.data(), errorInfo)) {
        m_audioTrack.reset(nullptr);
        qmlWarning(this) << "AudioTrack initialize failed:" << errorInfo;
        errorInfo.reset();
    }

    FFMS_DestroyIndex(index);
}

void MediaClip::componentComplete()
{
    m_componentComplete = true;
    if (clipStart() < 0)
        setClipStart(0);
    loadMedia();
    if (clipEnd() < 0) {
        setClipEnd(std::max(m_audioTrack ? m_audioTrack->duration() : 0, m_videoTrack ? m_videoTrack->duration() : 0));
    }
    m_currentFrameTime = Interval(milliseconds(clipStart()), milliseconds(clipStart()) + MediaManager::singletonInstance()->session()->frameDuration());
}