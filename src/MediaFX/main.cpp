// Copyright (C) 2023 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "encoder.h"
#include "session.h"
#include <QByteArray>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QMessageLogContext>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUrl>
extern "C" {
#include <libavutil/parseutils.h>
#include <libavutil/rational.h>
}
#ifdef EVENTLOGGER
#include "event_logger.h"
#endif
using namespace Qt::Literals::StringLiterals;

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

    app.setOrganizationDomain(u"mediafx.org"_s);
    app.setOrganizationName(u"MediaFX"_s);
    app.setApplicationName(u"MediaFX"_s);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"MediaFX\nCopyright (C) 2023-2024 Andrew Wason\nSPDX-License-Identifier: GPL-3.0-or-later"_s);
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addOption({ { u"f"_s, u"fps"_s }, u"Output frames per second, can be integer or rational e.g. 30000/1001."_s, u"fps"_s, u"30"_s });
    parser.addOption({ { u"r"_s, u"sampleRate"_s }, u"Output audio sample rate (Hz)"_s, u"sampleRate"_s, u"44100"_s });
    parser.addOption({ { u"s"_s, u"size"_s }, u"Output video frame size, WxH."_s, u"size"_s, u"640x360"_s });
    parser.addOption({ { u"w"_s, u"exitOnWarning"_s }, u"Exit on QML warnings."_s });
    parser.addPositionalArgument(u"source"_s, u"QML source URL."_s);
    parser.addPositionalArgument(u"output"_s, u"Output nut video path (or '-' for stdout)."_s);

    parser.process(app);

    AVRational frameRate { 0 };
    if (av_parse_video_rate(&frameRate, parser.value(u"fps"_s).toUtf8()) < 0)
        parser.showHelp(1);
    int width = 0, height = 0;
    if (av_parse_video_size(&width, &height, parser.value(u"size"_s).toUtf8()) < 0)
        parser.showHelp(1);
    QSize frameSize(width, height);

    int sampleRate = parser.value(u"sampleRate"_s).toInt();

    const QStringList args = parser.positionalArguments();
    if (args.size() != 2)
        parser.showHelp(1);

    QUrl url(args.at(0));
    QString output(args.at(1));
    if (output == u"-"_s)
        output = u"pipe:"_s;

    Encoder encoder(output, frameSize, frameRate, sampleRate);
    if (!encoder.isValid()) {
        qCritical("Failed to initialize encoder");
        return 1;
    }
    Session session(&encoder, url, parser.isSet(u"exitOnWarning"_s));
    if (!session.isValid()) {
        qCritical("Failed to initialize session");
        return 1;
    }
    return app.exec();
}
