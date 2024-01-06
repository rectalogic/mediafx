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

#include "application.h"
#include "encoder.h"
#ifdef EVENTLOGGER
#include "event_logger.h"
#endif
#include "session.h"
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMessageLogContext>
#include <QString>
#include <QStringList>
#include <QStringLiteral>
#include <QUrl>

const auto ffmpegPreamble = qSL("-f rawvideo -video_size ${MEDIAFX_FRAMESIZE} -pixel_format rgb0 -framerate ${MEDIAFX_FRAMERATE} -i pipe:${MEDIAFX_VIDEOFD}");

int main(int argc, char* argv[])
{
#ifdef TARGET_OS_MAC
    putenv(
        const_cast<char*>("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM=1"));
#endif

    initializeMediaFX();
    QGuiApplication app(argc, argv);

#ifdef EVENTLOGGER
    app.installEventFilter(new EventLogger(&app));
#endif

    app.setOrganizationDomain(qSL("mediafx.stream"));
    app.setOrganizationName(qSL("mediaFX"));
    app.setApplicationName(qSL("mediaFX"));

    QCommandLineParser parser;
    parser.setApplicationDescription(qSL("mediaFX"));
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addOption({ { qSL("f"), qSL("fps") }, qSL("Output frames per second, can be float or rational e.g. 30000/1001"), qSL("fps"), qSL("30") });
    parser.addOption({ { qSL("s"), qSL("size") }, qSL("Output video frame size, WxH"), qSL("size"), qSL("640x360") });
    parser.addOption({ { qSL("o"), qSL("output") }, qSL("Output filename"), qSL("output") });
    parser.addOption({ { qSL("c"), qSL("command") }, qSL("Encoder commandline"), qSL("command") });
    parser.addPositionalArgument(qSL("source"), qSL("QML source URL."));

    parser.process(app);

    Encoder::FrameRate frameRate = Encoder::FrameRate::parse(parser.value(qSL("fps")));
    if (frameRate.isEmpty())
        parser.showHelp(1);

    Encoder::FrameSize frameSize = Encoder::FrameSize::parse(parser.value(qSL("size")));
    if (frameSize.isEmpty())
        parser.showHelp(1);

    QString command;
    if (parser.isSet(qSL("command"))) {
        command = parser.value(qSL("command"));
        if (command == qSL("ffplay")) {
            // XXX need to handle audio eventually
            command = qSL("ffplay -autoexit -infbuf ") + ffmpegPreamble;
        } else if (command.startsWith(qSL("ffmpeg:"))) {
            if (!parser.isSet(qSL("output"))) {
                parser.showHelp(1);
            }
            if (command == qSL("ffmpeg:lossless")) {
                command = qSL("ffmpeg ") + ffmpegPreamble + qSL(" -f nut -vcodec ffv1 -flags bitexact -g 1 -level 3 -pix_fmt rgb32 -fflags bitexact -y ${MEDIAFX_OUTPUT}");
            } else if (command == qSL("ffmpeg:mov")) {
                command = qSL("ffmpeg ") + ffmpegPreamble + qSL(" -f mov -vcodec rawvideo -pix_fmt uyvy422 -vtag yuvs -y ${MEDIAFX_OUTPUT}");
            } else {
                parser.showHelp(1);
            }
        }
    }
    QString output;
    if (parser.isSet(qSL("output"))) {
        output = parser.value(qSL("output"));
    }

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        qCritical("Missing required source url");
        parser.showHelp(1);
    }
    QUrl url(args.at(0));

    Encoder encoder(frameSize, frameRate);
    if (!encoder.initialize(output, command)) {
        qCritical("Failed to initialize encoder");
        return 1;
    }
    Session session(&encoder);
    if (!session.initialize(url)) {
        qCritical("Failed to initialize session");
        return 1;
    }
    return app.exec();
}
