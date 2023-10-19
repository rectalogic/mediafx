// Copyright (c) 2023 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "visual_clip.h"
#include <QImage>
#include <QObject>
#include <QVideoFrame>
#include <QtQmlIntegration>
class QUrl;

class ImageClip : public VisualClip {
    Q_OBJECT
    QML_ELEMENT

public:
    using VisualClip::VisualClip;

protected:
    void loadMedia(const QUrl&) override;

    bool prepareNextVideoFrame() override;

    void setActive(bool active) override;

    void stop() override;

private:
    QImage image;
    QVideoFrame videoFrame;
};