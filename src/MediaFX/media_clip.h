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

#pragma once

#include "interval.h"
#include <QObject>
#include <QQmlParserStatus>
#include <QUrl>
#include <QtQmlIntegration>
#include <QtTypes>
#include <memory>
class AudioTrack;
class VideoTrack;

class MediaClip : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QUrl source READ source WRITE setSource REQUIRED FINAL)
    Q_PROPERTY(int clipStart READ clipStart WRITE setClipStart NOTIFY clipStartChanged FINAL)
    Q_PROPERTY(int clipEnd READ clipEnd WRITE setClipEnd NOTIFY clipEndChanged FINAL)
    Q_PROPERTY(int clipDuration READ clipDuration NOTIFY clipDurationChanged FINAL)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(Interval clipCurrentTime READ clipCurrentTime NOTIFY clipCurrentTimeChanged FINAL)
    QML_ELEMENT

signals:
    void clipStartChanged();
    void clipEndChanged();
    void clipDurationChanged();
    void activeChanged(bool);
    void clipCurrentTimeChanged();
    void clipEnded();

public:
    using QObject::QObject;

    explicit MediaClip(QObject* parent = nullptr);
    ~MediaClip();

    QUrl source() const { return m_source; };
    void setSource(const QUrl&);

    qint64 clipStart() const { return m_clipStart; };
    void setClipStart(qint64 ms);

    qint64 clipEnd() const { return m_clipEnd; };
    void setClipEnd(qint64 ms);

    qint64 clipDuration() const { return m_clipEnd - m_clipStart; };

    const Interval& clipCurrentTime() const { return m_currentFrameTime; };

    void setActive(bool active);
    bool isActive() const { return m_active; };

    void render();

    void classBegin() override {};
    void componentComplete() override;

    void updateActive();

protected:
    void loadMedia();

    bool isComponentComplete() { return m_componentComplete; };

    VideoTrack* videoTrack() { return m_videoTrack.get(); };
    AudioTrack* audioTrack() { return m_audioTrack.get(); };

    friend class MediaAttached;

private:
    bool m_componentComplete = false;
    bool m_active = false;
    QUrl m_source;
    qint64 m_clipStart;
    qint64 m_clipEnd;

    Interval m_currentFrameTime;

    std::unique_ptr<VideoTrack> m_videoTrack;
    std::unique_ptr<AudioTrack> m_audioTrack;
};
