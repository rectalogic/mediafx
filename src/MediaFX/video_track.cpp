// Copyright (C) 2023 Andrew Wason
// SPDX-License-Identifier: GPL-3.0-or-later

#include "video_track.h"
#include "error_info.h"
#include "interval.h"
#include "media_clip.h"
#include <QDebug>
#include <QList>
#include <QQmlInfo>
#include <QSize>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoSink>
#include <ffms.h>
#include <libavutil/pixfmt.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

QVideoFrameFormat::ColorTransfer colorTransfer(const FFMS_Frame* frameProperties)
{
    // This is AVFrame::color_trc. See QFFmpegVideoBuffer::colorTransfer()
    switch (frameProperties->TransferCharateristics) {
    case AVCOL_TRC_BT709:
    case AVCOL_TRC_BT1361_ECG:
    case AVCOL_TRC_BT2020_10:
    case AVCOL_TRC_BT2020_12:
    case AVCOL_TRC_SMPTE240M:
        return QVideoFrameFormat::ColorTransfer_BT709;
    case AVCOL_TRC_GAMMA22:
    case AVCOL_TRC_SMPTE428:
    case AVCOL_TRC_IEC61966_2_1:
    case AVCOL_TRC_IEC61966_2_4:
        return QVideoFrameFormat::ColorTransfer_Gamma22;
    case AVCOL_TRC_GAMMA28:
        return QVideoFrameFormat::ColorTransfer_Gamma28;
    case AVCOL_TRC_SMPTE170M:
        return QVideoFrameFormat::ColorTransfer_BT601;
    case AVCOL_TRC_LINEAR:
        return QVideoFrameFormat::ColorTransfer_Linear;
    case AVCOL_TRC_SMPTE2084:
        return QVideoFrameFormat::ColorTransfer_ST2084;
    case AVCOL_TRC_ARIB_STD_B67:
        return QVideoFrameFormat::ColorTransfer_STD_B67;
    default:
        break;
    }
    return QVideoFrameFormat::ColorTransfer_Unknown;
}

QVideoFrameFormat::ColorSpace colorSpace(const FFMS_Frame* frameProperties)
{
    // This is AVFrame::colorspace. See QFFmpegVideoBuffer::colorSpace()
    switch (frameProperties->ColorSpace) {
    default:
    case AVCOL_SPC_UNSPECIFIED:
    case AVCOL_SPC_RESERVED:
    case AVCOL_SPC_FCC:
    case AVCOL_SPC_SMPTE240M:
    case AVCOL_SPC_YCGCO:
    case AVCOL_SPC_SMPTE2085:
    case AVCOL_SPC_CHROMA_DERIVED_NCL:
    case AVCOL_SPC_CHROMA_DERIVED_CL:
    case AVCOL_SPC_ICTCP:
        return QVideoFrameFormat::ColorSpace_Undefined;
    case AVCOL_SPC_RGB:
        return QVideoFrameFormat::ColorSpace_AdobeRgb;
    case AVCOL_SPC_BT709:
        return QVideoFrameFormat::ColorSpace_BT709;
    case AVCOL_SPC_BT470BG:
    case AVCOL_SPC_SMPTE170M:
        return QVideoFrameFormat::ColorSpace_BT601;
    case AVCOL_SPC_BT2020_NCL:
    case AVCOL_SPC_BT2020_CL:
        return QVideoFrameFormat::ColorSpace_BT2020;
    }
}

QVideoFrameFormat::ColorRange colorRange(const FFMS_Frame* frameProperties)
{
    // This is AVFrame::color_range. See QFFmpegVideoBuffer::colorRange()
    switch (frameProperties->ColorRange) {
    case AVCOL_RANGE_MPEG:
        return QVideoFrameFormat::ColorRange_Video;
    case AVCOL_RANGE_JPEG:
        return QVideoFrameFormat::ColorRange_Full;
    default:
        return QVideoFrameFormat::ColorRange_Unknown;
    }
}

bool VideoTrack::initialize(FFMS_Index* index, const char* sourceFile, ErrorInfo& errorInfo)
{
    int videoTrackNum = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_VIDEO, &errorInfo);
    if (videoTrackNum >= 0) {
        FFMS_VideoSource* videoSource = FFMS_CreateVideoSource(sourceFile, videoTrackNum, index, 1, mediaClip()->startTime() == 0 ? FFMS_SEEK_LINEAR_NO_RW : FFMS_SEEK_NORMAL, &errorInfo);
        if (videoSource) {
            const FFMS_Frame* frameProperties = FFMS_GetFrame(videoSource, 0, &errorInfo);
            if (frameProperties) {
                int pixelFormats[2] = { FFMS_GetPixFmt("rgba"), -1 };
                if (FFMS_SetOutputFormatV2(videoSource, pixelFormats, frameProperties->EncodedWidth, frameProperties->EncodedHeight, FFMS_RESIZER_BICUBIC, &errorInfo) == 0) {
                    m_videoSource = videoSource;
                    m_track = FFMS_GetTrackFromVideo(m_videoSource);
                    m_nextFrameInfo = FFMS_GetFrameInfo(m_track, m_nextFrameNum);
                    m_timebase = FFMS_GetTimeBase(m_track);
                    QVideoFrameFormat format = formatForFrame(frameProperties);
                    m_videoFrame.first = QVideoFrame(format);
                    m_videoFrame.second = QVideoFrame(format);
                    // Prime first frames data
                    if (mediaClip()->startTime() == 0) {
                        mapVideoFrameData(m_videoFrame.first, frameProperties);
                    }
                    updateActive();
                    return true;
                }
            }
        }
    }
    return false;
}

QVideoFrameFormat VideoTrack::formatForFrame(const FFMS_Frame* frameProperties) const
{
    QVideoFrameFormat format(QSize(frameProperties->EncodedWidth, frameProperties->EncodedHeight), QVideoFrameFormat::Format_RGBA8888);
    format.setColorTransfer(colorTransfer(frameProperties));
    format.setColorSpace(colorSpace(frameProperties));
    format.setColorRange(colorRange(frameProperties));
    return format;
}

void VideoTrack::mapVideoFrameData(QVideoFrame& videoFrame, const FFMS_Frame* frameProperties)
{
    QVideoFrameFormat format = videoFrame.surfaceFormat();
    if (format.colorTransfer() != colorTransfer(frameProperties)
        || format.colorSpace() != colorSpace(frameProperties)
        || format.colorRange() != colorRange(frameProperties)) {
        format = formatForFrame(frameProperties);
        videoFrame = QVideoFrame(format);
    }
    videoFrame.map(QVideoFrame::WriteOnly);
    memcpy(videoFrame.bits(0), frameProperties->Data[0], videoFrame.mappedBytes(0));
    videoFrame.unmap();
}

qint64 VideoTrack::calculateFrameStartTime(const FFMS_FrameInfo* frameInfo) const
{
    return (qint64)((frameInfo->PTS * m_timebase->Num) / (double)m_timebase->Den);
}

qint64 VideoTrack::duration() const
{
    if (!m_videoSource)
        return 0;
    const FFMS_VideoProperties* videoProps = FFMS_GetVideoProperties(m_videoSource);
    return round(videoProps->LastEndTime * 1000);
}

void VideoTrack::addVideoSink(QVideoSink* videoSink)
{
    if (!m_videoSinks.contains(videoSink)) {
        m_videoSinks.append(videoSink);
        updateActive();
    }
}

void VideoTrack::removeVideoSink(const QVideoSink* videoSink)
{
    if (m_videoSinks.removeOne(videoSink)) {
        updateActive();
    }
}

void VideoTrack::updateActive()
{
    setActive(!videoSinks().isEmpty() && m_videoSource);
}

void VideoTrack::render(const Interval& frameTime)
{
    auto frameInfo = m_nextFrameInfo;
    while (frameInfo && calculateFrameStartTime(frameInfo) < frameTime.start()) {
        m_nextFrameNum++;
        frameInfo = FFMS_GetFrameInfo(m_track, m_nextFrameNum);
    }
    // EOF if no frame info, continue sending last frame rendered
    if (frameInfo && frameInfo != m_nextFrameInfo) {
        m_nextFrameInfo = frameInfo;
        if (isActive()) {
            ErrorInfo errorInfo;
            const FFMS_Frame* frameProperties = FFMS_GetFrame(m_videoSource, m_nextFrameNum, &errorInfo);
            if (!frameProperties) {
                qmlWarning(mediaClip()) << "FFMS_GetFrame failed on frame" << m_nextFrameNum << errorInfo;
                stop();
                return;
            }
            // Swap frames because VideoOutput just compares internal frame pointers and ignores if the same
            std::swap(m_videoFrame.first, m_videoFrame.second);
            mapVideoFrameData(m_videoFrame.first, frameProperties);
        }
    }

    // We consume frames above if inactive since audio may be active. Only send frames if active.
    if (isActive()) {
        for (auto videoSink : videoSinks()) {
            videoSink->setVideoFrame(m_videoFrame.first);
        }
    }
}

void VideoTrack::stop()
{
    if (m_videoSource)
        FFMS_DestroyVideoSource(m_videoSource);
    m_videoSource = nullptr;
    m_track = nullptr;
    m_timebase = nullptr;
    m_videoFrame.first = QVideoFrame();
    m_videoFrame.second = QVideoFrame();
    updateActive();
}
