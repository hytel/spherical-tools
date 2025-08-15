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

#include "LPRecorder.h"

LPRecorder::LPRecorder() :
    m_is_recording(false),
    m_video_writer(nullptr),
    m_record_fps(24.0) {
}

void LPRecorder::beginRecording(const QString &output_video_path, int width, int height) {

    int scaled_width = width * 1;
    int scaled_height = height * 1;

    scaled_width = (scaled_width / 2) * 2;
    scaled_height = (scaled_height / 2) * 2;

#ifdef USE_OPENCV
    cv::Size S = cv::Size(scaled_width, scaled_height);

    printf("Dim = %d, %d\n", scaled_width, scaled_height);

    m_recording_width = scaled_width;
    m_recording_height = scaled_height;

    m_video_writer = new cv::VideoWriter();
    if (!m_video_writer->open(output_video_path.toStdString(),
                              cv::VideoWriter::fourcc('a', 'v', 'c', '1'),
                              m_record_fps,
                              S,
                              true)) {
        delete m_video_writer;
        m_video_writer = nullptr;
        printf("Can't write video\n");
        return;
    }
#else
#endif

    printf("Recording\n");
    m_is_recording = true;
}

void LPRecorder::endRecording(void) {

    printf("Stopping recording\n");
    if (m_video_writer) {
        //m_video_writer->release();
        delete m_video_writer;
        m_video_writer = nullptr;
    }

    m_is_recording = false;
}

void LPRecorder::writeFrame(const cv::Mat &frame) {
#ifdef USE_OPENCV
    m_video_writer->write(frame);
#else
#endif
}

