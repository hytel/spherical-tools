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
#include "LPVideoOutput.h"

LPVideoOutput::LPVideoOutput() {

}

LPVideoOutput::~LPVideoOutput() {

}

bool LPVideoOutput::beginWrite(
    const QString &input_path,
    int frame_width,
    int frame_height,
    AVFormatContext *input_fmt_ctx,
    AVCodecContext  *input_video_codec_ctx,
    int input_video_stream_index,
    int input_audio_stream_index) {

    // Make sure input dimesions are divisible by two
    m_frame_width = (frame_width / 2) * 2;
    m_frame_height = (frame_height / 2) * 2;

    m_input_fmt_ctx = input_fmt_ctx;
    m_input_video_codec_ctx = input_video_codec_ctx;
    m_input_video_stream_index = input_video_stream_index;
    m_input_audio_stream_index = input_audio_stream_index;

    QFileInfo fileInfo(input_path);
    QString folder = fileInfo.path();
    QString baseName = fileInfo.completeBaseName(); // "x"
    QString extension = fileInfo.suffix();          // "ext"
    QString output_filename = folder + "/" + baseName + "_reframed." + extension;

    m_output_fmt_ctx = nullptr;
    if (avformat_alloc_output_context2(&m_output_fmt_ctx, nullptr, nullptr, output_filename.toStdString().c_str()) < 0) {
        printf("Could not create output context\n");
        reset();
        return false;
    }

    av_dict_set(&m_output_fmt_ctx->metadata, "title", "Hillel Steinberg's Spherical Qt Application Reframer", 0);

    m_output_video_codec = avcodec_find_encoder(AV_CODEC_ID_H264);  // or same as input
    if (m_output_video_codec == nullptr) {
        printf("Could not find output codec\n");
        reset();
        return false;
    }

    m_output_video_stream = avformat_new_stream(m_output_fmt_ctx, m_output_video_codec);
    if (m_output_video_stream == nullptr) {
        printf("Could not create output video stream\n");
        reset();
        return false;
    }

    m_output_video_enc_ctx = avcodec_alloc_context3(m_output_video_codec);
    if (m_output_video_enc_ctx == nullptr) {
        printf("Could not create output video encoder context\n");
        reset();
        return false;
    }

    av_opt_set(m_output_video_enc_ctx->priv_data, "preset", "slow", 0);
    av_opt_set(m_output_video_enc_ctx->priv_data, "profile", "high", 0);

    m_output_audio_stream = avformat_new_stream(m_output_fmt_ctx, nullptr);
    if (m_output_audio_stream == nullptr) {
        printf("Could not create output audio stream\n");
        reset();
        return false;
    }

    // Use same audio parameters for output as input
    avcodec_parameters_copy(m_output_audio_stream->codecpar, m_input_fmt_ctx ->streams[m_input_audio_stream_index]->codecpar);

    // Setup output video
    m_output_video_enc_ctx->width = m_frame_width;
    m_output_video_enc_ctx->height = m_frame_height;
    m_output_video_enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_output_video_enc_ctx->time_base = m_input_fmt_ctx->streams[m_input_video_stream_index]->time_base;

    AVRational input_frame_rate = av_guess_frame_rate(m_input_fmt_ctx, m_input_fmt_ctx->streams[m_input_video_stream_index], nullptr);
    m_output_video_enc_ctx->framerate = input_frame_rate;

    // Match video
    m_output_video_stream->r_frame_rate = input_frame_rate;
    m_output_video_stream->time_base = m_output_video_enc_ctx->time_base;

    // Match audio
    m_output_audio_stream->time_base = m_input_fmt_ctx->streams[m_input_audio_stream_index]->time_base;

    // Get video rate from input
    int ret_value = avcodec_open2(m_output_video_enc_ctx, m_output_video_codec, nullptr);
    if (ret_value < 0) {
        printf("Could not open output video codec\n");
        reset();
        return false;
    }
    avcodec_parameters_from_context(m_output_video_stream->codecpar, m_output_video_enc_ctx);

    // Read to open output file
    if (!(m_output_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_output_fmt_ctx->pb, output_filename.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
            printf("Could not create output video file\n");
            reset();
            return false;
        }
    }

    AVDictionary *format_options = nullptr;
    av_dict_set(&format_options, "movflags", "faststart", 0);

    if (avformat_write_header(m_output_fmt_ctx, nullptr) < 0) {
        printf("Could not write output video header\n");
        reset();
        return false;
    }

    m_output_packet = av_packet_alloc();
    m_filtered_frame = av_frame_alloc();
    m_filtered_frame->format = AV_PIX_FMT_YUV420P;
    m_filtered_frame->width = m_frame_width;
    m_filtered_frame->height = m_frame_height;
    av_frame_get_buffer(m_filtered_frame, 32);

    return true;
}

bool LPVideoOutput::writeFrame(int64_t input_frame_pts, const cv::Mat &cv_frame) {

    if (!m_output_sws) {
        m_output_sws = sws_getContext(
        cv_frame.cols, m_frame_height, AV_PIX_FMT_BGR24,
        m_frame_width, m_frame_height, this->m_output_video_enc_ctx->pix_fmt,
        SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    uint8_t* src_data[4] = { cv_frame.data, nullptr, nullptr, nullptr };
    size_t src_frame_step = cv_frame.step;
    int src_linesize[4] = { static_cast<int>(src_frame_step), 0, 0, 0 };

    if (sws_scale(m_output_sws,
                  src_data, src_linesize,
                  0, m_output_video_enc_ctx->height,
                  m_filtered_frame->data, m_filtered_frame->linesize) < 0) {
        printf("Could not scale frame for output\n");
        return false;
    }
    AVRational src_time_base = m_input_fmt_ctx->streams[m_input_video_stream_index]->time_base;
    AVRational enc_time_base = m_output_video_enc_ctx->time_base;

    m_filtered_frame->pts = av_rescale_q(input_frame_pts,
                                         src_time_base,
                                         enc_time_base);

    printf("PTS = %d\n", (int)m_filtered_frame->pts);

    //av_init_packet(m_output_packet);

    if (avcodec_send_frame(m_output_video_enc_ctx, m_filtered_frame) < 0) {
        printf("Could not encode frame!\n");
        return false;
    }

    while (avcodec_receive_packet(m_output_video_enc_ctx, m_output_packet) == 0) {
        m_output_packet->stream_index = 0;
        if (av_interleaved_write_frame(m_output_fmt_ctx, m_output_packet) < 0) {
            printf("Could not write output packet\n");
            printf("Packet written!\n");
        }
        av_packet_unref(m_output_packet);
    }

    //av_freep(&m_filtered_frame->data[0]);

    return true;
}

bool LPVideoOutput::saveAudioPacket(AVPacket *audio_packet) {

    audio_packet->stream_index = 1;
    if (av_interleaved_write_frame(m_output_fmt_ctx, audio_packet) < 0) {
        printf("Couldn't write audio packet\n");
        return false;
    }

    return true;
}

bool LPVideoOutput::endWrite(void) {

    avcodec_send_frame(m_output_video_enc_ctx, nullptr);

    while (avcodec_receive_packet(m_output_video_enc_ctx, m_output_packet) == 0) {
        m_output_packet->stream_index = 0;
        av_interleaved_write_frame(m_output_fmt_ctx, m_output_packet);
        av_packet_unref(m_output_packet);
    }

    if (av_write_trailer(m_output_fmt_ctx) < 0) {
        printf("Could not write output video trailler\n");
        return false;
    }

    reset();

    return true;
}

void LPVideoOutput::reset(void) {

    if (m_output_video_enc_ctx) {
        avcodec_free_context(&m_output_video_enc_ctx);
        m_output_video_enc_ctx = nullptr;
    }

    if (!(m_output_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_closep(&m_output_fmt_ctx->pb) < 0) {
            printf("Could not close output video file\n");
        }
    }

    if (m_output_fmt_ctx) {
        avformat_free_context(m_output_fmt_ctx);
        m_output_fmt_ctx = nullptr;
    }

    if (m_filtered_frame) {

        // Free frame data first
        //av_freep(&m_filtered_frame->data[0]);

        av_frame_free(&m_filtered_frame);
        m_filtered_frame = nullptr;
    }

    //av_frame_free(&filtered_frame);
    if (m_output_packet) {
        av_packet_free(&m_output_packet);
        m_output_packet = nullptr;
    }

    if (m_output_sws) {
        sws_freeContext(m_output_sws);
        m_output_sws = nullptr;
    }
}
