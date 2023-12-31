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

#include "session.h"
#include "animation.h"
#include "encoder.h"
#include "media_manager.h"
#include "render_control.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QMessageLogContext>
#include <QObject>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickView>
#include <QSGRendererInterface>
#include <QUrl>
#include <errno.h>
#include <ffms.h>
#include <string.h>
#include <unistd.h>
#ifdef MEDIAFX_ENABLE_VULKAN
#include <QQuickGraphicsConfiguration>
#endif

QEvent::Type Session::renderEventType = static_cast<QEvent::Type>(QEvent::registerEventType());

Session::Session(Encoder* encoder, QObject* parent)
    : QObject(parent)
    , encoder(encoder)
    , m_frameDuration(encoder->frameRate().toFrameDuration())
    , animationDriver(new AnimationDriver(m_frameDuration, this))
    , renderControl(nullptr)
    , quickView(nullptr)
{
    FFMS_Init(0, 0);
    connect(this, &Session::exitApp, qApp, &QCoreApplication::exit, Qt::QueuedConnection);

    animationDriver->install();
    renderControl.reset(new RenderControl());
    quickView.reset(new QQuickView(QUrl(), renderControl.get()));
#ifdef MEDIAFX_ENABLE_VULKAN
    if (quickView->rendererInterface()->graphicsApi() == QSGRendererInterface::Vulkan) {
        vulkanInstance.setExtensions(QQuickGraphicsConfiguration::preferredInstanceExtensions());
        vulkanInstance.create();
    }
#endif

    manager = new MediaManager(this, this);
    MediaManagerForeign::s_singletonInstance = manager;

    quickView->setResizeMode(QQuickView::ResizeMode::SizeRootObjectToView);
    quickView->resize(encoder->frameSize().toSize());
    connect(quickView.get(), &QQuickView::statusChanged, this, &Session::quickViewStatusChanged);
    connect(quickView->engine(), &QQmlEngine::warnings, this, &Session::engineWarnings);
}

Session::~Session()
{
    animationDriver->uninstall();
    FFMS_Deinit();
}

bool Session::initialize(const QUrl& url)
{
#ifdef MEDIAFX_ENABLE_VULKAN
    if (quickView->rendererInterface()->graphicsApi() == QSGRendererInterface::Vulkan) {
        if (!vulkanInstance.isValid()) {
            qCritical() << "Invalid Vulkan instance";
            return false;
        }
        quickView->setVulkanInstance(&vulkanInstance);
    }
#endif
    if (!renderControl->install(quickView.get())) {
        qCritical() << "Failed to install QQuickRenderControl";
        return false;
    }
    quickView->setSource(url);
    if (quickView->status() == QQuickView::Error)
        return false;
    else
        return true;
}

void Session::quickViewStatusChanged(QQuickView::Status status)
{
    if (status == QQuickView::Error) {
        emit exitApp(1);
    } else if (status == QQuickView::Ready) {
        QCoreApplication::postEvent(this, new QEvent(renderEventType));
    }
}

void Session::engineWarnings(const QList<QQmlError>& warnings)
{
    for (const QQmlError& warning : warnings) {
        qWarning() << warning.toString();
    }
}

bool Session::event(QEvent* event)
{
    if (event->type() == renderEventType) {
        render();
        return true;
    }
    return QObject::event(event);
}

int write(int fd, qsizetype size, const char* data)
{
    size_t bytesIO = 0;
    while (bytesIO < size) {
        ssize_t n = write(fd, data + bytesIO, size - bytesIO);
        if (n == -1) {
            qCritical() << "write failed: " << strerror(errno);
            return -1;
        }
        bytesIO = bytesIO + n;
    }
    return size;
}

void Session::render()
{
    manager->render();

    switch (manager->encodingState()) {
    case MediaManager::EncodingState::Encoding:
        break;
    case MediaManager::EncodingState::Stopped:
        return;
    case MediaManager::EncodingState::Stopping:
        manager->setEncodingState(MediaManager::EncodingState::Stopped);
        break;
    }

    auto frameData = renderControl->renderVideoFrame();

    if (write(encoder->videofd(), frameData.size(), frameData.constData()) == -1)
        return;

    animationDriver->advance();

    QCoreApplication::postEvent(this, new QEvent(renderEventType));
}