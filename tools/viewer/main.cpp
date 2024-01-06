/*
 * Copyright (C) 2024 Andrew Wason
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
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickView>
#include <QStringLiteral>
#include <QUrl>

int main(int argc, char* argv[])
{
    initializeMediaFX();
    QGuiApplication app(argc, argv);

    app.setOrganizationDomain(qSL("mediafx.stream"));
    app.setOrganizationName(qSL("mediaFX"));
    app.setApplicationName(qSL("mediaFX viewer"));

    QCommandLineParser parser;
    parser.setApplicationDescription(qSL("mediaFX viewer"));
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addPositionalArgument(qSL("source"), qSL("QML source URL."));

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        qCritical("Missing required source url");
        parser.showHelp(1);
    }
    QUrl url(args.at(0));

    QQuickView quickView(url);
    // qCritical() << quickView.errors();
    quickView.show();

    return app.exec();
}
