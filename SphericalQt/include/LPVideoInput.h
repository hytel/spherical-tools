#ifndef LP_VIDEO_INPUT_HPP
#define LP_VIDEO_INPUT_HPP

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

#ifdef USE_OPENCV
#include <opencv2/videoio.hpp>  // Video write
#include <opencv2/imgproc.hpp>

#else

// FFMPEG includes
extern "C" {
#include <libavformat/avformat.h>   // AVFormatContext, avformat_open_input, etc.
#include <libavcodec/avcodec.h>     // AVCodec, AVCodecContext, AVPacket, etc.
#include <libavutil/imgutils.h>     // av_image_alloc, av_image_fill_arrays, etc.
#include <libavutil/opt.h>          // av_opt_set, etc. (optional)
#include <libavutil/mathematics.h>  // for timestamp math (optional)
#include <libswscale/swscale.h>     // for image scaling (optional if converting to RGB)
}

#endif

// Forward declaration
class LPVideoOutput;

class LPVideoInput {

public:

    LPVideoInput();

#ifdef USE_OPENCV
    inline cv::Mat &getCurrentFrame(void) {
        return m_current_frame;
    }
#else
#endif

    inline bool isRecording(void) const {
        return m_is_recording;
    }

    // Public methods
    void begin(const QString &path, int rendered_width, int rendered_height);
    void reset(void);
    void play(void);
    void pause(void);
    void setPosition(double percentage);
    void setSpeed(double speed);
    bool update(bool force_frame);
    bool getCurrentFrame(QImage &return_frame_image);
    double getPlaybackPercentage(void);
    bool beginWrite(int rendered_width, int rendered_height);
    bool writeFrame(const cv::Mat &frame);
    bool endWrite(void);

private:

    // Private methods
    void convertFrameToRGB(void);

    QString          m_input_path;
    bool             m_is_paused = false;
    double           m_speed = 24.0;
    bool             m_has_video = false;
#ifdef USE_OPENCV
    cv::VideoCapture m_video_capture;
    cv::Mat          m_current_frame;
#else
    AVFormatContext *m_fmt_ctx = nullptr;
    AVCodecContext  *m_codec_ctx = nullptr;
    SwsContext      *m_sws_ctx = nullptr;
    int              m_video_stream_index = -1;
    int              m_audio_stream_index = -1;
    int              m_frame_width = 0;
    int              m_frame_height = 0;
    AVPacket        *m_packet = nullptr;
    AVFrame         *m_frame =  nullptr;
    int64_t          m_current_frame_pts = -1;
    int64_t          m_playback_pos = -1;
    int64_t          m_total_file_len = -1;
    uint8_t         *m_rgb_data = nullptr;
    int              m_rgb_stride = 0;
    uint8_t         *m_dest[1] = { 0 };
    int              m_dest_linesize[1] = { 0 };
    bool             m_receive_more_frames = true;
    QImage           m_current_frame;
#endif

    bool             m_is_recording = false;
    LPVideoOutput   *m_output_video = nullptr;
};

#endif // LP_VIDEO_INPUT_HPP
