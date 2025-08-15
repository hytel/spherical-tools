#ifndef LP_RECORDER_HPP
#define LP_RECORDER_HPP

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

// OpenCV includes
#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat)
#include <opencv2/videoio.hpp>  // Video write
#include <opencv2/imgproc.hpp>

class LPRecorder {

public:

    LPRecorder();

    inline void setFPS(double fps) {
        m_record_fps = fps;
    }

    inline double getFPS(void) const {
        return m_record_fps;
    }

    inline bool isRecording(void) const {
        return m_is_recording;
    }

    inline int getRecordingWidth(void) const {
        return m_recording_width;
    }

    inline int getRecordingHeight(void) const {
        return m_recording_height;
    }

    void beginRecording(const QString &output_video_path, int width, int height);
    void endRecording(void);
    void writeFrame(const cv::Mat &frame);

private:

    int              m_recording_width = 0;
    int              m_recording_height = 0;
    bool             m_is_recording = false;
    cv::VideoWriter *m_video_writer = nullptr;
    double           m_record_fps = 0.0;
};

#endif // LP_RECORDER_HPP
