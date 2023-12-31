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

#include "encoder.h"
#include <QDebug>
#include <QMessageLogContext>
#include <QString>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

Encoder::~Encoder()
{
    if (m_videofd != -1) {
        close(m_videofd);
        if (m_audiofd == m_videofd)
            m_audiofd = -1;
        m_videofd = -1;
    }
    if (m_audiofd != -1) {
        close(m_audiofd);
        m_audiofd = -1;
    }
    if (m_pid != -1)
        waitpid(m_pid, nullptr, 0);
}

bool Encoder::initialize(const QString& output, const QString& command)
{
    if (m_audiofd != -1 || m_videofd != -1)
        return true;

    if (output.isEmpty() && command.isEmpty()) {
        qCritical() << "neither command nor output specified";
        return false;
    }
    if (command.isEmpty()) {
        if (output == "-") {
            m_audiofd = STDOUT_FILENO;
            m_videofd = STDOUT_FILENO;
            return true;
        } else {
            int fd = open(output.toUtf8().constData(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd == -1) {
                qCritical() << "Failed to open" << output;
                return false;
            }
            m_audiofd = m_videofd = fd;
            return true;
        }
    }

    int audiofds[2];
    int videofds[2];

    if (pipe(videofds) == -1 || pipe(audiofds) == -1) {
        qCritical() << "pipe failed:" << strerror(errno);
        return false;
    }

    int pid = fork();
    if (pid == -1) {
        qCritical() << "fork failed:" << strerror(errno);
        return false;
    }

    // In the child
    if (pid == 0) {
        close(audiofds[1]);
        close(videofds[1]);

        setenv("MEDIAFX_VIDEOFD", QString::number(videofds[0]).toUtf8().constData(), 1);
        setenv("MEDIAFX_AUDIOFD", QString::number(audiofds[0]).toUtf8().constData(), 1);
        setenv("MEDIAFX_FRAMESIZE", frameSize().toString().toUtf8().constData(), 1);
        setenv("MEDIAFX_FRAMERATE", frameRate().toString().toUtf8().constData(), 1);
        if (!output.isEmpty()) {
            setenv("MEDIAFX_OUTPUT", output.toUtf8().constData(), 1);
        }
        auto commandBytes = command.toUtf8();
        char* const argv[] = { const_cast<char*>("sh"), const_cast<char*>("-c"), const_cast<char*>(commandBytes.constData()), nullptr };
        if (execvp(argv[0], argv) < 0) {
            qCritical() << "exec" << command << "failed:" << strerror(errno);
            exit(1);
        }
    }

    // In the parent

    m_pid = pid;
    m_audiofd = audiofds[1];
    close(audiofds[0]);
    m_videofd = videofds[1];
    close(videofds[0]);

    return true;
}
