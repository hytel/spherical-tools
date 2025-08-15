#ifndef LP_VIDEO_OUTPUT_HPP
#define LP_VIDEO_OUTPUT_HPP

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
#include <QtCore/QString>
#include <QtGui/QImage>

// OpenCV includes
#include <opencv2/core.hpp> // Basic OpenCV structures (cv::Mat)

extern "C" {
#include <libavformat/avformat.h>   // AVFormatContext, avformat_open_input, etc.
#include <libavcodec/avcodec.h>     // AVCodec, AVCodecContext, AVPacket, etc.
#include <libavutil/imgutils.h>     // av_image_alloc, av_image_fill_arrays, etc.
#include <libavutil/opt.h>          // av_opt_set, etc. (optional)
#include <libavutil/mathematics.h>  // for timestamp math (optional)
#include <libswscale/swscale.h>     // for image scaling (optional if converting to RGB)
}

class LPVideoOutput {

public:

    LPVideoOutput();
    virtual ~LPVideoOutput();

    // Inline methods
    inline int getFrameWidth(void) const {
        return m_frame_width;
    }

    inline int getFrameHeight(void) const {
        return m_frame_height;
    }

    // Public methods
    bool beginWrite(const QString &input_path,
                    int frame_width,
                    int frame_height,
                    AVFormatContext *input_fmt_ctx,
                    AVCodecContext  *input_video_codec_ctx,
                    int input_video_stream_index,
                    int input_audio_stream_index);
    bool writeFrame(int64_t input_frame_pts, const cv::Mat &cv_frame);
    bool saveAudioPacket(AVPacket *audio_packet);
    bool endWrite(void);
    void reset(void);

private:

    // Output parameters
    QString          m_output_path;
    int              m_frame_width = -1;
    int              m_frame_height = -1;
    AVFormatContext *m_output_fmt_ctx = nullptr;
    const AVCodec   *m_output_video_codec = nullptr;
    AVStream        *m_output_video_stream = nullptr;
    AVCodecContext  *m_output_video_enc_ctx = nullptr;
    AVStream        *m_output_audio_stream =  nullptr;
    AVPacket        *m_output_packet =  nullptr;
    AVFrame         *m_filtered_frame =  nullptr;
    SwsContext      *m_output_sws = nullptr;

    // Source input parameters
    AVFormatContext *m_input_fmt_ctx = nullptr;
    AVCodecContext  *m_input_video_codec_ctx = nullptr;
    int              m_input_video_stream_index = -1;
    int              m_input_audio_stream_index = -1;

};

#endif // LP_VIDEO_OUTPUT_HPP
