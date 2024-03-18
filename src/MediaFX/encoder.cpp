// Copyright (C) 2023 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * mux raw audio (float) and video (RGBA) streams into NUT format
 * We bypass encoding and just stuff our raw data into AVPacket.data for
 * the muxer since it is already in the correct format.
 * http://git.ffmpeg.org/gitweb/nut.git
 * https://ffmpeg.org/nut.html
 */

#include "encoder.h"
#include "formats.h"
#include "output_format.h"
#include "output_stream.h"
#include "util.h"
#include <QAudioBuffer>
#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QSize>
#include <QtLogging>
#include <stdint.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/packet.h>
#include <libavcodec/version.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/dict.h>
#include <libavutil/mathematics.h>
#include <libavutil/rational.h>
#include <libavutil/version.h>
}

// NOLINTBEGIN(bugprone-assignment-in-if-condition)

Encoder::Encoder(const QString& outputFile, const OutputFormat& outputFormat)
    : QObject()
    , m_outputFile(outputFile)
{
    int ret = 0;
    // Select nut format
    if ((ret = avformat_alloc_output_context2(&m_formatContext, nullptr, "nut", qUtf8Printable(m_outputFile))) < 0) {
        qCritical() << "Could not allocate an output context, error:" << av_err2qstring(ret);
        return;
    }

    // Video stream, AV_CODEC_ID_RAWVIDEO/AV_PIX_FMT_RGBA
    std::unique_ptr<OutputStream> video(new OutputStream(m_formatContext, AV_CODEC_ID_RAWVIDEO));
    if (!video->isValid())
        return;
    AVCodecContext* videoCodecContext = video->codecContext();
    videoCodecContext->pix_fmt = VideoPixelFormat_FFMPEG;
    videoCodecContext->width = outputFormat.frameSize().width();
    videoCodecContext->height = outputFormat.frameSize().height();
    AVRational timeBase(av_inv_q(outputFormat.frameRate()));
    int64_t gcd = av_gcd(FFABS(timeBase.num), FFABS(timeBase.den));
    if (gcd) {
        timeBase.num = FFABS(timeBase.num) / gcd;
        timeBase.den = FFABS(timeBase.den) / gcd;
    }
    video->stream()->time_base = videoCodecContext->time_base = timeBase;
    if (!video->open())
        return;

    // Audio stream, AV_CODEC_ID_PCM_F32(BE|LE)/AV_SAMPLE_FMT_FLT
    std::unique_ptr<OutputStream> audio(new OutputStream(m_formatContext, AudioCodec_FFMPEG));
    if (!audio->isValid())
        return;
    AVCodecContext* audioCodecContext = audio->codecContext();
    audioCodecContext->sample_fmt = AudioSampleFormat_FFMPEG;
    audioCodecContext->sample_rate = outputFormat.sampleRate();
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 37, 100)
    audioCodecContext->channel_layout = AudioChannelLayout_FFMPEG;
    audioCodecContext->channels = av_get_channel_layout_nb_channels(audioCodecContext->channel_layout);
#else
    if ((ret = av_channel_layout_copy(&audioCodecContext->ch_layout, &AudioChannelLayout_FFMPEG)) < 0) {
        qCritical() << "Could not copy channel layout, error:" << av_err2qstring(ret);
        return;
    }
#endif
    audio->stream()->time_base = audioCodecContext->time_base = (AVRational) { 1, audioCodecContext->sample_rate };
    if (!audio->open())
        return;

    m_audioStream.swap(audio);
    m_videoStream.swap(video);

    if (!(m_formatContext->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&m_formatContext->pb, qUtf8Printable(m_outputFile), AVIO_FLAG_WRITE)) < 0) {
            qCritical() << "Could not open output file" << m_outputFile << ", avio_open:" << av_err2qstring(ret);
            return;
        }
    }
    AVDictionary* opt = nullptr;
    if ((ret = av_dict_set(&opt, "fflags", "bitexact", 0)) < 0) {
        qCritical() << "Could not set options, av_dict_set:" << av_err2qstring(ret);
        return;
    }
    ret = avformat_write_header(m_formatContext, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        qCritical() << "Could not open output file, avio_open:" << av_err2qstring(ret);
        return;
    }

    m_isValid = true;
};

Encoder::~Encoder()
{
    m_audioStream.reset();
    m_videoStream.reset();
    if (m_formatContext)
        avformat_free_context(m_formatContext);
}

bool Encoder::encode(const QAudioBuffer& audioBuffer, const QByteArray& videoData)
{
    AVPacket* videoPacket = m_videoStream->packet();
    videoPacket->flags |= AV_PKT_FLAG_KEY;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-type-reinterpret-cast)
    videoPacket->data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(videoData.constData()));
    videoPacket->size = static_cast<int>(videoData.size());
    if (!m_videoStream->writePacket(m_formatContext, 1)) {
        emit encodingError();
        return false;
    }

    AVPacket* audioPacket = m_audioStream->packet();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    audioPacket->data = const_cast<uint8_t*>(audioBuffer.constData<uint8_t>());
    audioPacket->size = static_cast<int>(audioBuffer.byteCount());
    if (!m_audioStream->writePacket(m_formatContext, audioBuffer.frameCount())) {
        emit encodingError();
        return false;
    }

    return true;
}

bool Encoder::finish()
{
    int ret = 0;
    if ((ret = av_write_trailer(m_formatContext)) < 0) {
        qCritical() << "Could not write trailer, av_write_trailer:" << av_err2qstring(ret);
        emit encodingError();
        return false;
    }
    if (!(m_formatContext->flags & AVFMT_NOFILE)) {
        if ((ret = avio_closep(&m_formatContext->pb)) < 0) {
            qCritical() << "Could not close file, avio_closep:" << av_err2qstring(ret);
            emit encodingError();
            return false;
        }
    }
    return true;
}

// NOLINTEND(bugprone-assignment-in-if-condition)