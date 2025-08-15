#ifndef LPOPENGLWIDGET_H
#define LPOPENGLWIDGET_H

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
#include <QtCore/QMimeData>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtOpenGL/QOpenGLTexture>
#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtOpenGL/QOpenGLDebugLogger>
#include <QtOpenGL/QOpenGLDebugLogger>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtWidgets/QFileDialog>

// OpenCV includes
#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat)
#include <opencv2/videoio.hpp>  // Video write
#include <opencv2/imgproc.hpp>

// QtSpherical includes
#include "LPMainWindow.h"
#include "LPVideoInput.h"

#define MAX_LUTS
#define MAX_SLIDER_VALUE 10000

class LPOpenGLWidget;

enum LPSource {
    None,
    Image,
    Video
};

class LPOpenGLWidget : public QOpenGLWidget {

public:

    LPOpenGLWidget(QWidget *parent = nullptr);

    inline void setApp(QApplication *app) {
        m_app = app;
    }

    inline void setScale(float scale) {
        m_scale = scale;
        repaint();
    }

    inline void setFilterStrength(float filter_strength) {
        m_filter_strength = filter_strength;
        repaint();
    }

    inline void setLUTByIndex(int ndx) {
        m_current_lut = ndx;
        repaint();
    }

    inline void updateTransform(void) {
        QQuaternion zRotation = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, m_rotate_z);
        QQuaternion all_rot = zRotation * QQuaternion::fromEulerAngles(m_rotate_x, m_rotate_y, 0.0);
        m_transformMat = all_rot.toRotationMatrix();
    }

    inline void setRotateX(float angle) {
        m_rotate_x = angle;
        updateTransform();
        doPaint();
    }

    inline void setRotateY(float angle) {
        m_rotate_y = angle;
        updateTransform();
        doPaint();
    }

    inline void setRotateZ(float angle) {
        m_rotate_z = angle;
        updateTransform();
        doPaint();
    }

    inline void setShiftX(float shift) {
        m_shift_x = shift;
        doPaint();
    }

    inline void setShiftY(float shift) {
        m_shift_y = shift;
        doPaint();
    }

    inline void setGamma(float gamma) {
        m_gamma = gamma;
        doPaint();
    }

    inline void setBrightness(float brightness) {
        m_brightness = brightness;
        doPaint();
    }

    inline void setSaturation(float saturation) {
        m_saturation = saturation;
        doPaint();
    }

    inline void setTemperature(float temperature) {
        m_temperature = temperature;
        doPaint();
    }

    inline void setVignetteIntensity(float vignette_intensity) {
        m_vignette_intensity = vignette_intensity;
        doPaint();
    }

    inline void setVignetteExtent(float vignette_extent) {
        m_vignette_extent = vignette_extent;
        doPaint();
    }

    //inline void setFPS(double fps) {
    //    m_recorder.setFPS(fps);
    //}

    //inline const LPRecorder &getRecorder(void) const {
    //    return m_recorder;
    //}

    inline const cv::Mat &getLastFrame(void) const {
        return m_last_frame;
    }

    inline bool getRequestQuit(void) const {
        return m_request_quit;
    }

    inline bool isRecording(void) const {
        //return m_recorder.isRecording();
        return m_video_input.isRecording();
    }

    inline bool isPaused(void) const {
        return m_is_paused;
    }

    inline LPSource getSource(void) const {
        return m_source;
    }

    void setUI(LPMainWindow *main_window, Ui::LPMainWindow *ui, Ui::LPSettingsDialog *settings_ui);
    void advanceFrame(void);
    void frameDone(void);
    void handleLoggedMessage(const QOpenGLDebugMessage &debugMessage);
    bool loadFile(const QString &filename);
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    //bool processVideoFrame(const QVideoFrame &frame);
    void timelineChanged(int pos);
    void save(void);
    void renderImageDirectly(const QImage &img);
    void toggleRecord(void);
    void setSpeed(double speed);
    void cycleLUT(int direction);
    const QString &getCurrentFilterString(void) const;
    void resetSettings(void);
    void doPaint(void);
    bool beginWrite(void);

    // Actions
    void playbackPositionChanged(qint64 position);
    //void mediaStateChanged(QMediaPlayer::State newState);

    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:

    void processRecording(void);

    QApplication              *m_app = nullptr;
    LPMainWindow              *m_main_window = nullptr;
    Ui::LPMainWindow          *m_ui = nullptr;
    Ui::LPSettingsDialog      *m_settings_ui = nullptr;
    QString                    m_filename;
    QImage                     m_img;

    QOpenGLTexture            *m_frame_texture = nullptr;
    QVector<QOpenGLTexture *>  m_lut_textures;
    QOpenGLShaderProgram       m_shader_program;
    QOpenGLDebugLogger        *m_opengl_logger = nullptr;

    LPSource                   m_source;

    LPVideoInput               m_video_input;

    int                        m_vertexLocation = 0;
    int                        matrixLocation = 0;
    int                        m_imageTextureLocation = 0;
    int                        m_tcoordsLocation = 0;

    float                      m_scale = 1.0f;
    float                      m_filter_strength = 1.0f;
    float                      m_aspect = 0.0f;

    float                      m_rotate_x = 0.0f;
    float                      m_rotate_y = 0.0f;
    float                      m_rotate_z = 0.0f;

    float                      m_shift_x = 0.0f;
    float                      m_shift_y = 0.0f;

    float                      m_gamma = 0.0f;
    float                      m_brightness = 0.0f;
    float                      m_saturation = 0.0f;
    float                      m_temperature = 0.0f;

    float                      m_vignette_intensity = 0.0f;
    float                      m_vignette_extent = 0.0f;

    bool                       m_file_changed = false;
    bool                       m_is_paused = false;
    bool                       m_mouse_left_down = false;
    bool                       m_shift_pressed = false;
    bool                       m_force_frame = false;
    bool                       m_animate_heading = false;
    bool                       m_animate_pitch = false;
    bool                       m_animate_roll = false;
    bool                       m_loaded_luts = false;

    int                        m_last_mouse_x = 0;
    int                        m_last_mouse_y = 0;

    QMatrix3x3                 m_transformMat;

    LPOpenGLWidget            *m_save_widget = nullptr;

    qint64                     m_last_frame_time = 0;
    cv::Mat                    m_last_frame;

    int                        m_current_lut = 0;

    bool                       m_request_quit = false;
};

#endif // LPOPENGLWIDGET_H
