// Copyright (C) 2024 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "render_session.h"
#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>
#include <QQuickWindow>
#include <QtCore>
#include <QtQmlIntegration>
#include <memory>
#include <qtgui-config.h>
#if (QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>))
#define MEDIAFX_ENABLE_VULKAN
#endif
#ifdef MEDIAFX_ENABLE_VULKAN
#include <QVulkanInstance>
#endif
class QAudioBuffer;
class RenderControl;

class RenderWindow : public QQuickWindow, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(RenderSession* renderSession READ renderSession WRITE setRenderSession NOTIFY renderSessionChanged REQUIRED FINAL)
    QML_ELEMENT

public:
    using QQuickWindow::QQuickWindow;

    explicit RenderWindow();
    RenderWindow(RenderWindow&&) = delete;
    RenderWindow& operator=(RenderWindow&&) = delete;
    ~RenderWindow() override;

    RenderSession* renderSession() const { return m_renderSession; }
    void setRenderSession(RenderSession* renderSession);

signals:
    void renderSessionChanged();
    void frameReady(const QAudioBuffer& audioBuffer, const QByteArray& videoData);

public slots:
    void render();

protected:
    void classBegin() override { }
    void componentComplete() override;

private:
    Q_DISABLE_COPY(RenderWindow);

    RenderWindow(RenderControl* renderControl);

    QPointer<RenderSession> m_renderSession;
#ifdef MEDIAFX_ENABLE_VULKAN
    QVulkanInstance m_vulkanInstance;
#endif
    std::unique_ptr<RenderControl> m_renderControl;
    bool m_isValid = false;
};
