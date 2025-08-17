/*-----------------------------------------------------------------------------
The MIT License

Copyright © 2025-present Hillel Steinberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------*/

// Qt includes
#include <QFileInfo>

// Qt Spherical includes
#include "LPVideoInput.h"
#include "LPVideoOutput.h"

LPVideoInput::LPVideoInput() :
    //m_total_frame_num(0.0),
    //m_current_frame_num(0.0),
    m_is_paused(true),
    m_speed(24.0),
    m_has_video(false),
    m_output_video(new LPVideoOutput())
{
}

void LPVideoInput::begin(const QString &path, int rendered_width, int rendered_height) {

    m_has_video = false;

    if (m_fmt_ctx)
        reset();

    if (avformat_open_input(&m_fmt_ctx, path.toStdString().c_str(), nullptr, nullptr) < 0) {
        throw std::runtime_error("Could not open file");
        reset();
        return;
    }

    if (avformat_find_stream_info(m_fmt_ctx, nullptr) < 0) {
        throw std::runtime_error("Could not find stream info");
        reset();
        return;
    }

    // Find video
    for (unsigned int i = 0; i < m_fmt_ctx->nb_streams; i++) {
        if (m_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_video_stream_index = i;
            break;
        }
    }

    // Find audio
    for (unsigned int i = 0; i < m_fmt_ctx->nb_streams; i++) {
        if (m_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audio_stream_index = i;
            break;
        }
    }

    if (m_video_stream_index == -1) {
        throw std::runtime_error("Could not find video stream");
        reset();
        return;
    }

    AVCodecParameters *codecpar = m_fmt_ctx->streams[m_video_stream_index]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        throw std::runtime_error("Unsupported codec");
        reset();
        return;
    }

    m_codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(m_codec_ctx, codecpar);

    m_codec_ctx->thread_count = 0;
    m_codec_ctx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;

    if (avcodec_open2(m_codec_ctx, codec, nullptr) < 0) {
        throw std::runtime_error("Could not open codec");
        reset();
        return;
    }

    m_frame_width = m_fmt_ctx->streams[m_video_stream_index]->codecpar->width;
    m_frame_height = m_fmt_ctx->streams[m_video_stream_index]->codecpar->height;

    // Also make sure output will work
    //if (!m_output_video->beginWrite(path, rendered_width, rendered_height, m_fmt_ctx, m_codec_ctx, m_video_stream_index, m_audio_stream_index)) {
    //    throw std::runtime_error("Could not create output file");
    //    reset();
    //    return;
    //}

    m_sws_ctx = sws_getContext(
        m_frame_width,
        m_frame_height,
        static_cast<AVPixelFormat>(m_fmt_ctx->streams[m_video_stream_index]->codecpar->format),  // input format
        m_frame_width,
        m_frame_height,
        AV_PIX_FMT_RGB24,              // output format
        SWS_BILINEAR,
        nullptr, nullptr, nullptr);

    m_input_path = path;

    int rgb_bytes_per_pixel = 3; // for RGB24

    m_rgb_stride = m_frame_width * rgb_bytes_per_pixel;
    m_rgb_data = new uint8_t[m_rgb_stride * m_frame_height];

    m_dest[0] = { m_rgb_data };
    m_dest_linesize[0] = { m_rgb_stride };

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();

    //m_total_frame_num = m_fmt_ctx->streams[m_video_stream_index]->nb_frames;

    m_has_video = true;

    m_playback_pos = 0;

    m_total_file_len = QFileInfo(m_input_path).size();

    play();
}

bool LPVideoInput::beginWrite(int rendered_width, int rendered_height) {
    bool status = m_output_video->beginWrite(
        m_input_path,
        rendered_width,
        rendered_height,
        m_fmt_ctx,
        m_codec_ctx,
        m_video_stream_index,
        m_audio_stream_index);

    m_is_recording = status;

    return status;
}

bool LPVideoInput::writeFrame(const cv::Mat &frame) {
    m_output_video->writeFrame(m_current_frame_pts, frame);
    return true;
}

bool LPVideoInput::endWrite(void) {

    bool end_status = m_output_video->endWrite();
    if (end_status)
        m_is_recording = false;

    return end_status;
}

void LPVideoInput::reset(void) {

    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }

    if (m_packet) {
        av_packet_free(&m_packet);
        m_packet = nullptr;
    }

    if (m_codec_ctx) {
        avcodec_free_context(&m_codec_ctx);
        m_codec_ctx = nullptr;
    }

    if (m_fmt_ctx) {
        avformat_close_input(&m_fmt_ctx);
        m_fmt_ctx = nullptr;
    }

    if (m_sws_ctx) {
        sws_freeContext(m_sws_ctx);
        m_sws_ctx = nullptr;
    }

    m_playback_pos = -1;
    m_total_file_len = -1;
}

void LPVideoInput::play(void) {
    m_is_paused = false;
}


void LPVideoInput::pause(void) {
    m_is_paused = true;
}

void LPVideoInput::setPosition(double percentage) {

    // No video loaded
    if (!m_fmt_ctx)
        return;

    if (percentage < 0)
        percentage = 0;
    else if (percentage > 1.0)
        percentage = 1.0;

    m_receive_more_frames = false;

    if (!(m_fmt_ctx->pb->seekable & AVIO_SEEKABLE_NORMAL)) {
        printf("Input is not seekable by byte.\n");
    }

    int64_t file_size = avio_size(m_fmt_ctx->pb);
    if (file_size <= 0) {
        throw std::runtime_error("File size not known or not seekable");
    }

    int64_t target_byte = static_cast<int64_t>(percentage * file_size);

    int ret_status = av_seek_frame(m_fmt_ctx, -1, target_byte, AVSEEK_FLAG_BYTE);
    if (ret_status < 0) {

        // Plan B

        // Byte seek failed, use time as a measurement instead
        AVStream* stream = m_fmt_ctx->streams[m_video_stream_index];

        int64_t target_ts = static_cast<int64_t>(percentage * stream->duration);

        if (av_seek_frame(m_fmt_ctx, m_video_stream_index, target_ts, AVSEEK_FLAG_BACKWARD) < 0) {
            fprintf(stderr, "Error seeking\n");
        }
    }

    avcodec_flush_buffers(m_codec_ctx);

    m_playback_pos = (int64_t)(m_total_file_len * percentage);

    m_is_paused = false;
}

void LPVideoInput::setSpeed(double speed) {
    m_speed = speed;
}

double LPVideoInput::getPlaybackPercentage(void) {

    if (m_playback_pos < 0)
        return 0.0;

    return (double)(m_playback_pos) / (double)(m_total_file_len);
}

void LPVideoInput::convertFrameToRGB(void) {

    sws_scale(
        m_sws_ctx,
        m_frame->data,
        m_frame->linesize,
        0,
        m_frame_height,
        m_dest,
        m_dest_linesize
        );

    // Wrap frame as QImage
    m_current_frame =
        QImage(m_rgb_data, m_frame_width, m_frame_height, m_rgb_stride, QImage::Format_RGB888, nullptr, m_rgb_data);

        //QImage(m_rgb_data, m_frame_width, m_frame_height, m_rgb_stride, QImage::Format_RGB888,
        //          [](void* ptr) { delete[] static_cast<uint8_t*>(ptr); }, m_rgb_data);
}

bool LPVideoInput::update(bool force_frame) {

    if (!m_has_video || (m_is_paused && !force_frame))
        return false;

    // Any outstanding frames to receive from last time?
    if (m_receive_more_frames) {

        if (avcodec_receive_frame(m_codec_ctx, m_frame) == 0) {

            // Frame is ready – use it!m_
            //printf("Frame: %d x %d\n", m_frame->width, m_frame->height);
            convertFrameToRGB();
            return true;
        }

        m_receive_more_frames = false;
    }

    while (av_read_frame(m_fmt_ctx, m_packet) >= 0) {

        m_playback_pos = m_packet->pos;

        if (m_packet->stream_index == m_video_stream_index) {

            // Send packet to decoder
            if (avcodec_send_packet(m_codec_ctx, m_packet) == 0) {

                // Read all frames for this packet
                m_receive_more_frames = true;

                while (avcodec_receive_frame(m_codec_ctx, m_frame) == 0) {

                    // Frame is ready – use it!
                    //printf("Frame: %d x %d\n", m_frame->width, m_frame->height);
                    m_current_frame_pts = m_frame->pts;
                    convertFrameToRGB();
                    return true;
                }
            }
        }
        else if (m_packet->stream_index == m_audio_stream_index && m_is_recording) {
            m_output_video->saveAudioPacket(m_packet);
        }

        av_packet_unref(m_packet);
    }

    printf("End of video\n");

    m_is_paused = true;
    return false;
}

bool LPVideoInput::getCurrentFrame(QImage &return_frame_image) {

    return_frame_image = m_current_frame;

    return true;
}
