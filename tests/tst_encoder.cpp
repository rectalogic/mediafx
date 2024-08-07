// Copyright (C) 2023 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encoder.h"
#include "render_context.h"
#include "util.h"
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QSysInfo>
#include <QtCore>
#include <QtTest>
#include <chrono>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
extern "C" {
#include <libavutil/rational.h>
}
using namespace std::chrono;

class tst_Encoder : public QObject {
    Q_OBJECT

private slots:
    void frameRate()
    {
        QCOMPARE(frameRateToFrameDuration<microseconds>(AVRational { 30, 1 }), 33333us);
    }

    void encode()
    {
        // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
        QString uname(QSysInfo::kernelType());
        uname.replace(0, 1, uname[0].toUpper());
        QDir outputDir(QFINDTESTDATA("../"));
        QString buildDir = "build/" + uname + "/output";
        outputDir.mkpath(buildDir);
        outputDir.setPath(outputDir.absoluteFilePath(buildDir));

        QFile encodedFile(outputDir.filePath("encoder.nut"));

        Encoder encoder;
        QSignalSpy spy(&encoder, SIGNAL(encodingError()));
        QVERIFY(spy.isValid());
        encoder.setOutputFileName(encodedFile.fileName());
        encoder.setFrameSize(QSize(160, 120));
        encoder.setFrameRate(Rational { 5, 1 });
        encoder.setSampleRate(44100);
        encoder.initialize();
        QVERIFY(spy.empty());

        QAudioFormat audioFormat;
        audioFormat.setSampleFormat(QAudioFormat::Float);
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigStereo);
        audioFormat.setSampleRate(encoder.sampleRate());
        QAudioBuffer audioBuffer(audioFormat.framesForDuration(frameRateToFrameDuration<microseconds>(encoder.frameRate()).count()), audioFormat);

        QByteArray videoData(static_cast<qsizetype>(encoder.frameSize().width() * encoder.frameSize().height() * 4), Qt::Uninitialized);

        const int frames = static_cast<const int>(2.0 * av_q2d(encoder.frameRate())); // 2 seconds
        double audioTime = 0;
        double audioIncr = 2 * M_PI * 110.0 / encoder.sampleRate();
        double audioIncr2 = 2 * M_PI * 110.0 / encoder.sampleRate() / encoder.sampleRate();

        for (int i = 0; i < frames; i++) {
            // Audio sine wave
            float* q = audioBuffer.data<float>();
            for (int j = 0; j < audioBuffer.frameCount(); j++) {
                float v = static_cast<float>(sin(audioTime));
                for (int c = 0; c < audioFormat.channelCount(); c++)
                    *q++ = v; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                audioTime += audioIncr;
                audioIncr += audioIncr2;
            }
            // Video RGBA colors
            int step = static_cast<int>(av_q2d(encoder.frameRate()) * i);
            for (int y = 0; y < encoder.frameSize().height(); y++) {
                for (int x = 0; x < encoder.frameSize().width(); x++) {
                    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    uint8_t* pixel = reinterpret_cast<uint8_t*>(&(videoData.data()[static_cast<ptrdiff_t>((y * encoder.frameSize().width() + x) * 4)]));
                    pixel[0] = x + y + step * 3;
                    pixel[1] = 128 + y + step * 2;
                    pixel[2] = 64 + x + step * 5;
                    pixel[3] = 0xff; // alpha
                    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
            }

            QVERIFY(encoder.encode(audioBuffer, videoData));
        }
        QVERIFY(encoder.finish());
        QVERIFY(spy.empty());

        QVERIFY(encodedFile.open(QIODevice::ReadOnly));
        QByteArray encodedData(encodedFile.readAll());
        encodedFile.close();

        QString fixturePath = QFINDTESTDATA("fixtures/output/mediafx-qt/encoder.nut");
        QFile fixtureFile(fixturePath);
        QVERIFY(fixtureFile.open(QIODevice::ReadOnly));
        QByteArray fixtureData(fixtureFile.readAll());
        fixtureFile.close();

        QCOMPARE(fixtureData, encodedData);
        // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
    }
};

QTEST_APPLESS_MAIN(tst_Encoder);
#include "tst_encoder.moc"