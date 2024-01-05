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
#include <QJSEngine>
#include <QList>
#include <QObject>
#include <QtAssert>
#include <QtQml>
#include <QtQmlIntegration>
class MediaClip;
class Session;

class MediaManager : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;
    MediaManager(Session* session, QObject* parent = nullptr);

    static MediaManager* singletonInstance();

    Q_INVOKABLE Interval createInterval(qint64 start, qint64 end)
    {
        return Interval(start, end);
    };

    Session* session() { return m_session; };

    void registerClip(MediaClip* clip);
    void unregisterClip(MediaClip* clip);

    void render();

    enum EncodingState {
        Encoding,
        Stopping,
        Stopped,
    };

    EncodingState encodingState() const { return m_encodingState; };
    void setEncodingState(EncodingState state) { m_encodingState = state; };

signals:
    void finishEncoding(bool immediate = true);

private:
    Session* m_session;
    QList<MediaClip*> activeClips;
    EncodingState m_encodingState = Encoding;
};

// https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON
struct MediaManagerForeign {
    Q_GADGET
    QML_FOREIGN(MediaManager)
    QML_SINGLETON
    QML_NAMED_ELEMENT(MediaManager)
public:
    inline static MediaManager* s_singletonInstance = nullptr;

    static MediaManager* create(QQmlEngine*, QJSEngine* engine)
    {
        // The instance has to exist before it is used. We cannot replace it.
        Q_ASSERT(s_singletonInstance);

        // The engine has to have the same thread affinity as the singleton.
        Q_ASSERT(engine->thread() == s_singletonInstance->thread());

        // There can only be one engine accessing the singleton.
        if (s_engine)
            Q_ASSERT(engine == s_engine);
        else
            s_engine = engine;

        // Explicitly specify C++ ownership so that the engine doesn't delete
        // the instance.
        QJSEngine::setObjectOwnership(s_singletonInstance, QJSEngine::CppOwnership);
        return s_singletonInstance;
    }

private:
    inline static QJSEngine* s_engine = nullptr;
};