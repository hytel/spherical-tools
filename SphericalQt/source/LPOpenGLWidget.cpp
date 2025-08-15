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
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Compatibility>

// OpenCV includes
#include <opencv2/highgui.hpp>

// Qt Spherical includes
#include "LPOpenGLWidget.h"

// Qt UI includes
#include "ui_LPMainWindow.h"
#include "ui_LPSettingsDialog.h"

static QStringList lut_names = {
    "default",
    "1977",
    "xproii",
    "amaro",
    "gotham",
    "hefe",
    "hudson",
    "lo-fi",
    "mayfair",
    "nashville",
    "sutro",
    "velencia",
    "willow",
    "Bleach_Bypass",
    "Bleak",
    "Candle_Light",
    "Foggy_Night",
    "Horror",
    "Late_Night",
    "Leave_Blue",
    "Leave_Green",
    "Leave_Red",
    "Sunset",
    "Teal_Orange",
    "Teal_Orange_Contrast",
    "Teal_Orange_Low_Contrast",
    "Vintage",
    "ab1",
    "ab2",
    "ab4",
    "ab6",
    "ab7",
    "ab8",
    "ab9",
    "ab10",
    "ab11",
    "ab15",
    "vertical",
    "yellowish",
    "infra-false-color",
    "hypersthene",
    "howlite",
    "hilutite",
    "hiddenite",
    "heulandite",
    "herderite",
    "hackmanite",
    "solarize",
    "fuji_reala_500d_kodak_2393",
    "tension_green",
    "fuji_f125_kodak_2395",
    "teal_orange_plus_contrast",
    "fuji_f125_kodak_2393",
    "night_from_day",
    "fuji_eterna_250d_kodak_2395",
    "moonlight",
    "fuji_eterna_250d_fuji_3510",
    "late_sunset",
    "foggy_Night2",
    "kodak_5295_fuji_3510",
    "filmstock_50",
    "kodak_5218_kodak_2395",
    "edgy_amber",
    "kodak_5218_kodak_2383",
    "drop_blues",
    "horror_blue",
    "futuristic_bleak",
    "candlelight",
    "film_default"
};

#ifdef JUNK
bool LPVideoOutput::present(const QVideoFrame &frame) {
    m_opengl_widget->processVideoFrame(frame);
    return true;
}

QList<QVideoFrame::PixelFormat> LPVideoOutput::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType ) const {

    if (handleType == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32_Premultiplied
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_RGB24
                << QVideoFrame::Format_RGB565
                << QVideoFrame::Format_RGB555
                << QVideoFrame::Format_ARGB8565_Premultiplied
                << QVideoFrame::Format_BGRA32
                << QVideoFrame::Format_BGRA32_Premultiplied
                << QVideoFrame::Format_BGR32
                << QVideoFrame::Format_BGR24
                << QVideoFrame::Format_BGR565
                << QVideoFrame::Format_BGR555
                << QVideoFrame::Format_BGRA5658_Premultiplied
                << QVideoFrame::Format_AYUV444
                << QVideoFrame::Format_AYUV444_Premultiplied
                << QVideoFrame::Format_YUV444
                << QVideoFrame::Format_YUV420P
                << QVideoFrame::Format_YV12
                << QVideoFrame::Format_UYVY
                << QVideoFrame::Format_YUYV
                << QVideoFrame::Format_NV12
                << QVideoFrame::Format_NV21
                << QVideoFrame::Format_IMC1
                << QVideoFrame::Format_IMC2
                << QVideoFrame::Format_IMC3
                << QVideoFrame::Format_IMC4
                << QVideoFrame::Format_Y8
                << QVideoFrame::Format_Y16
                << QVideoFrame::Format_Jpeg
                << QVideoFrame::Format_CameraRaw
                << QVideoFrame::Format_AdobeDng;
    }
    else {
        return QList<QVideoFrame::PixelFormat>();
    }
}
#endif

//--------------------------------------------------------------------------- LPOpenWidget ------------------------------------------------------------------------------------------

LPOpenGLWidget::LPOpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_main_window(nullptr),
    m_ui(nullptr),
    m_frame_texture(nullptr),
    m_source(None),
    m_scale(1.0f),
    m_filter_strength(1.0f),
    m_aspect(1.0f),
    m_rotate_x(0.0f),
    m_rotate_y(0.0f),
    m_rotate_z(0.0f),
    m_shift_x(0.0f),
    m_shift_y(0.0f),
    m_gamma(1.0f),
    m_brightness(0.0f),
    m_saturation(0.5),
    m_temperature(6500.0),
    m_vignette_intensity(15),
    m_vignette_extent(0.0f),
    m_file_changed(false),
    m_is_paused(false),
    m_mouse_left_down(false),
    m_shift_pressed(false),
    m_force_frame(false),
    m_animate_heading(false),
    m_animate_pitch(false),
    m_animate_roll(false),
    m_loaded_luts(false),
    m_last_mouse_x(0),
    m_last_mouse_y(0),
    m_save_widget(nullptr),
    m_last_frame_time(0),
    m_current_lut(0),
    m_request_quit(false) {

    m_transformMat.setToIdentity();

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setVersion(4, 3);
    //format.setVersion(4, 4);
    format.setOption(QSurfaceFormat::DebugContext, true);
    format.setOption(QSurfaceFormat::DeprecatedFunctions, true);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    setFormat(format); // must be called before the widget or its parent window gets shown

    QSizePolicy size_policy(QSizePolicy::Policy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(size_policy);
    setMinimumSize(QSize(1,1));

    m_opengl_logger = new QOpenGLDebugLogger(this);
}

void LPOpenGLWidget::setUI(LPMainWindow *main_window, Ui::LPMainWindow *ui, Ui::LPSettingsDialog *settings_ui) {

    m_main_window = main_window;
    m_ui = ui;
    m_settings_ui = settings_ui;

    connect(m_settings_ui->m_video_timeline_horizontal_slider, &QSlider::valueChanged, this, &LPOpenGLWidget::timelineChanged);
}

void LPOpenGLWidget::frameDone(void) {

    if (m_animate_roll) {
        int shift = m_settings_ui->m_rotate_z_horizontal_slider->value();
        shift += 14;
        if (shift > MAX_SLIDER_VALUE)
            shift = MAX_SLIDER_VALUE;
        m_settings_ui->m_rotate_z_horizontal_slider->setValue(shift);
    }
}

void LPOpenGLWidget::advanceFrame(void) {

    if (m_source != Video)
        return;

    if (m_video_input.update(m_force_frame)) {

#ifdef USE_OPENCV
        cv::Mat &current_frame = m_video_input.getCurrentFrame();
        cv::cvtColor(current_frame, current_frame, cv::COLOR_BGR2RGB);
        m_img = QImage((uchar*) current_frame.data, current_frame.cols, current_frame.rows, current_frame.step, QImage::Format_RGB888);
#else
        m_video_input.getCurrentFrame(m_img);
#endif

        m_file_changed = true;

        double percentage = m_video_input.getPlaybackPercentage();

        int timeline_pos = (int)(percentage * MAX_SLIDER_VALUE);
        if (timeline_pos > MAX_SLIDER_VALUE) {
            timeline_pos = MAX_SLIDER_VALUE;
        }

        if (m_settings_ui->m_video_timeline_horizontal_slider->value() != timeline_pos) {
            m_settings_ui->m_video_timeline_horizontal_slider->blockSignals(true);
            m_settings_ui->m_video_timeline_horizontal_slider->setValue(timeline_pos);
            m_settings_ui->m_video_timeline_horizontal_slider->blockSignals(false);
        }
    }

    m_force_frame = false;
}

void LPOpenGLWidget::resetSettings(void) {

    m_settings_ui->m_scale_horizontal_slider->setValue(100);

    m_settings_ui->m_rotate_x_horizontal_slider->setValue(0);
    m_settings_ui->m_rotate_y_horizontal_slider->setValue(2500);
    m_settings_ui->m_rotate_z_horizontal_slider->setValue(0);

    m_settings_ui->m_shift_x_horizontal_slider->setValue(5000);
    m_settings_ui->m_shift_y_horizontal_slider->setValue(5000);

    m_settings_ui->m_gamma_horizontal_slider->setValue(1000);
    m_settings_ui->m_brightness_horizontal_slider->setValue(5000);
    m_settings_ui->m_saturation_horizontal_slider->setValue(5000);
    m_settings_ui->m_temperature_horizontal_slider->setValue(6500);

    m_settings_ui->m_vignette_intensity_horizontal_slider->setValue(500);
    m_settings_ui->m_vignette_extent_horizontal_slider->setValue(0);

    m_current_lut = 0;
}

#ifdef JUNK
void LPOpenGLWidget::mediaStateChanged(QMediaPlayer::State newState) {

    // If we just stopped and we are recording, end the recording
    if (newState == QMediaPlayer::StoppedState && m_recorder.isRecording()) {
        record(false);
    }
}
#endif

void LPOpenGLWidget::timelineChanged(int pos) {

    double percentage = (double)pos / (double)MAX_SLIDER_VALUE;
    m_video_input.setPosition(percentage);
}

void LPOpenGLWidget::handleLoggedMessage(const QOpenGLDebugMessage &debugMessage) {
    qInfo() << debugMessage.message();
}

void LPOpenGLWidget::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void LPOpenGLWidget::dropEvent(QDropEvent *e) {

    foreach (const QUrl &url, e->mimeData()->urls()) {
        loadFile(url.toLocalFile());
        break;
    }
}

void LPOpenGLWidget::resizeGL(int w, int h) {

    glViewport(0, 0, w, h);
}

void LPOpenGLWidget::initializeGL() {

    m_shader_program.addShaderFromSourceCode(QOpenGLShader::Vertex,
          "uniform highp mat4 view_matrix;\n"
          "attribute highp vec4 vertex;\n"
          "attribute highp vec2 tcoords;\n"
          "varying vec2 tcoord;\n"
          "varying vec4 world_pos;\n"
          "void main(void)\n"
          "{\n"
          "   tcoord = tcoords;\n"
          "   world_pos = vertex;\n"
          "   gl_Position = view_matrix * vertex;\n"
          "}");

    QString fragment_src =  //"uniform mediump vec4 color;\n"
            "varying vec2 tcoord;\n"
            "varying vec4 world_pos;\n"
            "uniform vec2 dimension_vec;\n"
            "uniform sampler2D tex;\n"
            "uniform sampler2D lut;\n"
            "uniform mat3 transform;\n"
            "uniform float scale;\n"
            "uniform float brightness;\n"
            "uniform float gamma;\n"
            "uniform float saturation;\n"
            "uniform float temperature;\n"
            "uniform float vignette_intensity;\n"
            "uniform float vignette_extent;\n"
            "uniform float filter_strength;\n"
            "uniform float aspect;\n"
            "uniform float shift_x;\n"
            "uniform float shift_y;\n"
            "vec3 rgb2hsv(vec3 c)\n"
            "{\n"
                "vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
                "vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
                "vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"
                "float d = q.x - min(q.w, q.y);\n"
                "float e = 1.0e-10;\n"
                "return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
            "}\n"
            "vec3 hsv2rgb(vec3 c)\n"
            "{\n"
            "    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
            "    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
            "    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
            "}\n"
            "vec3 colorTemperatureToRGB(const in float temperature) {\n"
            "mat3 m = (temperature <= 6500.0) ? mat3(vec3(0.0, -2902.1955373783176, -8257.7997278925690),\n"
            "                                        vec3(0.0, 1669.5803561666639, 2575.2827530017594),\n"
            "                                        vec3(1.0, 1.3302673723350029, 1.8993753891711275)) : \n"
            "				                 		 mat3(vec3(1745.0425298314172, 1216.6168361476490, -8257.7997278925690),\n"
            "                                        vec3(-2666.3474220535695, -2173.1012343082230, 2575.2827530017594),\n"
            "                                        vec3(0.55995389139931482, 0.70381203140554553, 1.8993753891711275));\n"
            "return mix(clamp(vec3(m[0] / (vec3(clamp(temperature, 1000.0, 40000.0)) + m[1]) + m[2]), vec3(0.0), vec3(1.0)), vec3(1.0), smoothstep(1000.0, 0.0, temperature));\n"
            "}\n"
            "void main(void)\n"
            "{\n"
              "const float lut_size = 64.0;\n"
              "float lutsqr = sqrt(lut_size);\n"
              "const float PI = 3.14159265359;\n"
              "vec2 rads = vec2(PI * 2., PI);\n"
              "vec2 pnt = (tcoord - .5) * vec2(scale, scale * aspect) + vec2(shift_x, shift_y);\n"

              // Project to Sphere
              "float x2y2 = pnt.x * pnt.x + pnt.y * pnt.y;\n"
              "vec3 sphere_pnt = vec3(2. * pnt, x2y2 - 1.) / (x2y2 + 1.);\n"
              "sphere_pnt *= transform;\n"

              // Convert to Spherical Coordinates
              "float r = length(sphere_pnt);\n"
              "float lon = atan(sphere_pnt.y, sphere_pnt.x);\n"
              "float lat = acos(sphere_pnt.z / r);\n"

              //"gl_FragColor = vec4(texture2D(tex, (vec2(lon, lat) / rads)).rgb, 1.0);\n"
              "vec3 original_color = texture2D(tex, vec2(lon, lat) / rads).rgb;\n"

              // Gamma
              "original_color = clamp(pow(original_color, vec3(1.0/gamma)), 0.0, 1.0);\n"

              // Brightness
              "original_color = clamp(original_color + vec3(brightness), 0.0, 1.0);\n"

              // Vignette
              "vec2 uv = world_pos.xy / dimension_vec;\n"
              "uv *= (1.0 - uv.yx);\n"
              "float vig = uv.x*uv.y * vignette_intensity;\n" // Intensity
              "vig = pow(vig, vignette_extent);\n"            // Extent of vignette
              "original_color = clamp(original_color - vec3(1.0 - vig), 0.0, 1.0);\n"

              // Saturation
              "vec3 col_hsv = rgb2hsv(original_color);\n"
              "col_hsv.y = clamp(col_hsv.y * (saturation * 2.0), 0.0, 1.0);\n"
              "original_color = clamp(hsv2rgb(col_hsv), 0.0, 1.0);\n"

              // Temperature
              "original_color = mix(original_color, original_color * colorTemperatureToRGB(temperature), 1.0);\n"

              "vec3 original_color_scaled = floor(original_color * vec3(lut_size - 1.0));\n"
              "vec2 blue_index = vec2(mod(original_color_scaled.b, lutsqr), floor(original_color_scaled.b / lutsqr));\n"
              "vec2 lut_index = vec2((lut_size * blue_index.x + original_color_scaled.r) + 0.5, (lut_size * blue_index.y + original_color_scaled.g) + 0.5);\n"
              "gl_FragColor = vec4(mix(original_color, texture2D(lut, lut_index / 512.0).rgb, filter_strength), 1.0);\n"
            "}";

    m_shader_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragment_src);
    if (!m_shader_program.link()) {
        qWarning("ERROR linking shader");
    }
    if (!m_shader_program.bind())  {
        qWarning("ERROR binding shader");
    }

    m_vertexLocation = m_shader_program.attributeLocation("vertex");
    m_tcoordsLocation = m_shader_program.attributeLocation("tcoords");

    matrixLocation = m_shader_program.uniformLocation("view_matrix");
    m_imageTextureLocation = m_shader_program.uniformLocation("tex");

    bool has_debugging = context()->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
    if (!has_debugging) {
        qWarning("Missing debugging extension");
    }

    if (has_debugging) {
        if (!m_opengl_logger->initialize()) { // initializes in the current context, i.e. ctx
            qWarning("Could not initialize logging");
        }
    }

    connect(m_opengl_logger, &QOpenGLDebugLogger::messageLogged, this, &LPOpenGLWidget::handleLoggedMessage);

    if (has_debugging) {
        m_opengl_logger->startLogging();
    }

    // Load LUTs
    if (!m_loaded_luts) {

#ifdef Q_OS_MAC
        QString luts_path = QCoreApplication::applicationDirPath() + "/../Resources/luts/";
#else
        QString luts_path = QCoreApplication::applicationDirPath() + "/luts/";
#endif

        qInfo().noquote() << "Luts path: '" + luts_path + "'";

        for (int i = 0; i < lut_names.size(); ++i) {

            QString lut_path = luts_path + lut_names[i] + ".png";
            QImage lut_image(lut_path);

            if (lut_image.width() == 0)
                printf("Error: Couldn't load LUT: %s\n", lut_path.toStdString().c_str());

            QOpenGLTexture *lut_texture = new QOpenGLTexture(lut_image);

            lut_texture->setMinificationFilter(QOpenGLTexture::Linear); // MipMapLinear
            lut_texture->setMagnificationFilter(QOpenGLTexture::Linear);
            if (!lut_texture->create()) {
                qWarning() << (QString("ERROR Creating LUT Texture for: %1").arg(lut_path).toStdString().c_str());
                delete lut_texture;
            }
            else {
                m_lut_textures.push_back(lut_texture);
            }
        }

        m_loaded_luts = true;
    }
}

const QString &LPOpenGLWidget::getCurrentFilterString(void) const {

    return lut_names[m_current_lut];
}

void LPOpenGLWidget::paintGL(void) {

    if (!parent())
        printf("Save: Paint\n");

    advanceFrame();

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_file_changed && m_img.isNull())
        return;

    if (m_last_frame.empty() ||
        m_last_frame.cols != width() ||
        m_last_frame.rows != height()) {

        m_last_frame = cv::Mat(height() * 2, width() * 2, CV_8UC3);
    }

    m_aspect = (float)height() / (float)width();

    if (!parent())
        printf("Save: Aspect\n");

    if (m_file_changed) {

        if (m_frame_texture && m_frame_texture->isCreated()) {
            m_frame_texture->destroy();
        }

        // Load image from path if requested
        if (m_source == Image && !m_img.load(m_filename)) {
            //loads correctly
            qWarning("ERROR LOADING IMAGE");
        }

        if (m_frame_texture)
            m_frame_texture->setData(m_img);
        else
            m_frame_texture = new QOpenGLTexture(m_img);

        m_frame_texture->setMinificationFilter(QOpenGLTexture::Linear); // MipMapLinear
        m_frame_texture->setMagnificationFilter(QOpenGLTexture::Linear);
        if (!m_frame_texture->create()) {
            qWarning("ERROR Creating Frame Texture");
        }
        else {
            // qInfo("Frame Texture created!");
        }

        m_file_changed = false;
    }

    if (!m_frame_texture)
        return;

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(),0, height(), -1.0, 1.0); // left,right bottom,top near/far
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);


    glActiveTexture(GL_TEXTURE0);
    m_frame_texture->bind();

    glActiveTexture(GL_TEXTURE1);
    m_lut_textures[m_current_lut]->bind();

    // Draw the texture
    glPushMatrix();

    QMatrix4x4 pmvMatrix;

    pmvMatrix.ortho(rect());

    static GLfloat const quadTCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0
    };

    GLfloat const quadVertices[] = {
        0.0, 0.0, 0.0f,
        (float)width(), 0.0, 0.0f,
        (float)width(), (float)height(), 0.0f,
        0.0, (float)height(), 0.0f
    };

    QVector2D dimension_vec;

    dimension_vec.setX(width());
    dimension_vec.setY(height());

    GLenum error = glGetError();

    m_shader_program.enableAttributeArray(m_vertexLocation);
    m_shader_program.enableAttributeArray(m_tcoordsLocation);

    m_shader_program.setAttributeArray(m_vertexLocation, quadVertices, 3);
    error = glGetError();
    m_shader_program.setAttributeArray(m_tcoordsLocation, quadTCoords, 2);
    error = glGetError();

    m_shader_program.bind();
    error = glGetError();

    QSurfaceFormat sf = QOpenGLContext::currentContext()->format();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions(); //<QOpenGLFunctions_4_5_Compatibility>();
    f->initializeOpenGLFunctions();
    //f->glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, pmvMatrix.data());
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, pmvMatrix.data());
    error = glGetError();

    m_shader_program.setUniformValue(matrixLocation, pmvMatrix);
    //m_shader_program.setUniformValueArray(matrixLocation, &pmvMatrix, 1);
    error = glGetError();
    //const GLubyte *g = gluErrorString(error);

    m_shader_program.setUniformValue("dimension_vec", dimension_vec);
    m_shader_program.setUniformValue("scale", m_scale);
    m_shader_program.setUniformValue("gamma", m_gamma);
    m_shader_program.setUniformValue("brightness", m_brightness);
    m_shader_program.setUniformValue("saturation", m_saturation);
    m_shader_program.setUniformValue("temperature", m_temperature);
    m_shader_program.setUniformValue("vignette_intensity", m_vignette_intensity);
    m_shader_program.setUniformValue("vignette_extent", m_vignette_extent);
    m_shader_program.setUniformValue("filter_strength", m_filter_strength);
    m_shader_program.setUniformValue("aspect", m_aspect);

        error = glGetError();

    m_shader_program.setUniformValue("transform", m_transformMat);
    m_shader_program.setUniformValue("tex", 0);
    m_shader_program.setUniformValue("lut", 1);

    error = glGetError();

    float shift_x = (1.0f / width()) * (m_shift_x * width() * 1.5);
    m_shader_program.setUniformValue("shift_x", shift_x);

    float shift_y = (1.0f / height()) * (m_shift_y * height() * 1.5);
    m_shader_program.setUniformValue("shift_y", shift_y);

    glDrawArrays(GL_QUADS, 0, 4);

    m_shader_program.disableAttributeArray(m_tcoordsLocation);
    m_shader_program.disableAttributeArray(m_vertexLocation);

    glPopMatrix();

    m_frame_texture->release();
    m_lut_textures[m_current_lut]->release();

    glFlush();

    if (m_frame_texture && !m_last_frame.empty()) {

        size_t step = m_last_frame.step;
        int element_size = m_last_frame.elemSize();

        glPixelStorei(GL_PACK_ALIGNMENT, (step & 3) ? 1 : 4);

        //set length of one complete row in destination data (doesn't need to equal img.cols)
        glPixelStorei(GL_PACK_ROW_LENGTH, step / element_size);

        glReadPixels(0, 0, m_last_frame.cols, m_last_frame.rows, GL_BGR, GL_UNSIGNED_BYTE, m_last_frame.data);

        if (m_video_input.isRecording()) {

            static cv::Mat resized_img;
            if (1) {
                // Scale frame down
                m_last_frame(cv::Rect(0, 0, m_last_frame.cols / 2, m_last_frame.rows / 2)).copyTo(resized_img);

                // Mirror vertically
                cv::flip(resized_img, resized_img, 0);
            }
            else {
                cv::flip(m_last_frame, resized_img, 0);
            }

            m_video_input.writeFrame(resized_img);
        }
    }

    glEnd();

    if (error != GL_NO_ERROR) {
       qWarning("ERROR Creating Texture: %d\n", (int)error);
    }
}

bool LPOpenGLWidget::beginWrite(void) {
    m_video_input.beginWrite(width(), height());
    return true;
}

void LPOpenGLWidget::playbackPositionChanged(qint64 position) {

#ifdef JUNK
    qint64 total_duration = m_media_player.duration();

    int timeline_pos = 0;

    if (total_duration != 0) {
        double percentage = (double)position / (double)total_duration;
        timeline_pos = (int)(percentage * MAX_SLIDER_VALUE);
        if (timeline_pos > MAX_SLIDER_VALUE)
            timeline_pos = MAX_SLIDER_VALUE;
    }

    if (m_settings_ui->m_video_timeline_horizontal_slider->value() != timeline_pos) {
        m_settings_ui->m_video_timeline_horizontal_slider->blockSignals(true);
        m_settings_ui->m_video_timeline_horizontal_slider->setValue(timeline_pos);
        m_settings_ui->m_video_timeline_horizontal_slider->blockSignals(false);
    }
#endif

    double timeline_percentage = m_video_input.getPlaybackPercentage();
    int timeline_pos =  (int)(timeline_percentage * MAX_SLIDER_VALUE);
    if (timeline_pos > MAX_SLIDER_VALUE)
        timeline_pos = MAX_SLIDER_VALUE;

    if (m_settings_ui->m_video_timeline_horizontal_slider->value() != timeline_pos) {
        m_settings_ui->m_video_timeline_horizontal_slider->blockSignals(true);
        m_settings_ui->m_video_timeline_horizontal_slider->setValue(timeline_pos);
        m_settings_ui->m_video_timeline_horizontal_slider->blockSignals(false);
    }
}

bool LPOpenGLWidget::loadFile(const QString &filename) {

    QFileInfo file_info(filename);

    // First stop any media playing if it is
    m_video_input.reset();

    m_settings_ui->m_video_timeline_horizontal_slider->setValue(0);

    QString extension = file_info.suffix().toLower();
    if (extension == "mp4" || extension == "mov") {

        m_video_input.begin(filename, width(), height());

        m_settings_ui->m_video_timeline_horizontal_slider->setEnabled(true);
        m_is_paused = false;
        m_source = Video;
        m_file_changed = false;
    }
    else {
        m_settings_ui->m_video_timeline_horizontal_slider->setEnabled(false);
        m_source = Image;
        m_file_changed = true;
    }

    m_filename = filename;
    m_last_frame_time = 0;

    m_main_window->setWindowTitle(QString("Little Planet Renderer: %1").arg(m_filename));

    repaint();

    return true;
}

#ifdef JUNK
bool LPOpenGLWidget::processVideoFrame(const QVideoFrame &frame) {

    qint64 end_time = frame.endTime();

    m_last_frame_time = end_time;

    QVideoFrame cloneFrame(frame);

    m_img = frame.image();

    m_file_changed = true;

    doPaint();

    return true;
}
#endif

void LPOpenGLWidget::doPaint(void) {
    repaint();
}

void LPOpenGLWidget::save(void) {

    QString save_filename = QFileDialog::getSaveFileName(this,
          tr("Save PNG"), "",
          tr("PNG (*.png);;JPG (*.jpg);;DNG (*.dng);;TIF (*.tif);;All Files (*.*)"));

    if (save_filename.size() == 0)
        return;

    if (m_save_widget)
        delete m_save_widget;

    m_save_widget = new LPOpenGLWidget(nullptr);

    int output_width = 8192;
    int output_height = output_width * m_aspect;
    m_save_widget->setGeometry(0, 0, output_width, output_width * m_aspect);
    m_save_widget->setMinimumWidth(output_width);
    m_save_widget->setMaximumWidth(output_width);
    m_save_widget->setMinimumHeight(output_height);
    m_save_widget->setMaximumHeight(output_height);

    m_save_widget->resize(output_width, output_height);

    m_app->processEvents();

    m_save_widget->setRotateX(m_rotate_x);
    m_save_widget->setRotateY(m_rotate_y);
    m_save_widget->setRotateZ(m_rotate_z);

    m_save_widget->setShiftX(m_shift_x);
    m_save_widget->setShiftY(m_shift_y);

    m_save_widget->setScale(m_scale);

    m_save_widget->setGamma(m_gamma);
    m_save_widget->setBrightness(m_brightness);
    m_save_widget->setSaturation(m_saturation);
    m_save_widget->setTemperature(m_temperature);

    m_save_widget->setVignetteIntensity(m_vignette_intensity);
    m_save_widget->setVignetteExtent(m_vignette_extent);

    m_save_widget->setFilterStrength(m_filter_strength);

    m_save_widget->setLUTByIndex(m_current_lut);

    // Make sure the size takes hold
    m_file_changed = true;

    m_save_widget->move(-20000, -20000);

    m_save_widget->show();

    m_save_widget->renderImageDirectly(m_img);

    QImage current_image = m_save_widget->grabFramebuffer();
    current_image.save(save_filename);

    m_save_widget->hide();
}

void LPOpenGLWidget::renderImageDirectly(const QImage &img) {

    m_img = img.copy();
    m_file_changed = true;
    repaint();
    m_app->processEvents();
}


void LPOpenGLWidget::toggleRecord(void) {

    if (!m_video_input.isRecording()) {
        m_video_input.beginWrite(width(), height());
    }
    else {
        m_video_input.endWrite();
    }
}

void LPOpenGLWidget::mousePressEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton) {
        m_mouse_left_down = true;
    }

    m_last_mouse_x = event->position().x();
    m_last_mouse_y = event->position().y();
}

void LPOpenGLWidget::mouseReleaseEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton) {
        m_mouse_left_down = false;
    }

    m_last_mouse_x = event->position().x();
    m_last_mouse_y = event->position().y();
}

void LPOpenGLWidget::mouseDoubleClickEvent(QMouseEvent *) {

}

void LPOpenGLWidget::mouseMoveEvent(QMouseEvent *event) {

    // Get delta
    int delta_x = -(event->position().x() - m_last_mouse_x);
    int delta_y = event->position().y() - m_last_mouse_y;

    if (fabs(delta_x) >= 1) {
        int azimuth_value = m_settings_ui->m_rotate_x_horizontal_slider->value() + delta_x;
        while (azimuth_value > MAX_SLIDER_VALUE)
            azimuth_value -= MAX_SLIDER_VALUE;
        while (azimuth_value < 0)
            azimuth_value += MAX_SLIDER_VALUE;
        m_settings_ui->m_rotate_x_horizontal_slider->setValue(azimuth_value);
    }

    if (fabs(delta_y) >= 1) {
        int pitch_value = m_settings_ui->m_rotate_y_horizontal_slider->value() + delta_y;
        while (pitch_value > MAX_SLIDER_VALUE)
            pitch_value -= MAX_SLIDER_VALUE;
        while (pitch_value < 0)
            pitch_value += MAX_SLIDER_VALUE;
        m_settings_ui->m_rotate_y_horizontal_slider->setValue(pitch_value);
    }

    m_last_mouse_x = event->position().x();
    m_last_mouse_y = event->position().y();
}

void LPOpenGLWidget::keyPressEvent(QKeyEvent *event) {

    const int key = event->key();
    if (key == Qt::Key_Shift) {
        m_shift_pressed = true;
    }
    else if (key == Qt::Key_Space) {

        if (m_source == Video) {
            if (m_is_paused) {
                m_is_paused = false;
                //m_media_player.play();
                m_video_input.play();
            }
            else {
                m_is_paused = true;
                //m_media_player.pause();
                m_video_input.pause();
            }
        }
    }
    else if (key == Qt::Key_1) {
        m_animate_heading = !m_animate_heading;
    }
    else if (key == Qt::Key_2) {
        m_animate_pitch = !m_animate_pitch;
    }
    else if (key == Qt::Key_3) {
        m_animate_roll = !m_animate_roll;
    }
    else if (key == Qt::Key_Left) {
        int shift = m_settings_ui->m_shift_x_horizontal_slider->value();
        shift -= 5;
        if (shift < 0)
            shift = 0;
        m_settings_ui->m_shift_x_horizontal_slider->setValue(shift);
    }
    else if (key == Qt::Key_Right) {
        int shift = m_settings_ui->m_shift_x_horizontal_slider->value();
        shift += 5;
        if (shift > MAX_SLIDER_VALUE)
            shift = MAX_SLIDER_VALUE;
        m_settings_ui->m_shift_x_horizontal_slider->setValue(shift);
    }
    else if (key == Qt::Key_Up) {
        int shift = m_settings_ui->m_shift_y_horizontal_slider->value();
        shift -= 5;
        if (shift < 0)
            shift = 0;
        m_settings_ui->m_shift_y_horizontal_slider->setValue(shift);
    }
    else if (key == Qt::Key_Down) {
        int shift = m_settings_ui->m_shift_y_horizontal_slider->value();
        shift += 5;
        if (shift > MAX_SLIDER_VALUE)
            shift = MAX_SLIDER_VALUE;
        m_settings_ui->m_shift_y_horizontal_slider->setValue(shift);
    }
    else if (key == Qt::Key_BracketLeft) {
        int shift = m_settings_ui->m_rotate_z_horizontal_slider->value();
        shift -= 2;
        if (shift < 0)
            shift = 0;
        m_settings_ui->m_rotate_z_horizontal_slider->setValue(shift);
    }
    else if (key == Qt::Key_BracketRight) {
        int shift = m_settings_ui->m_rotate_z_horizontal_slider->value();
        shift += 2;
        if (shift > MAX_SLIDER_VALUE)
            shift = MAX_SLIDER_VALUE;
        m_settings_ui->m_rotate_z_horizontal_slider->setValue(shift);
    }
    else if (key == Qt::Key_C) {
        if (m_shift_pressed)
            m_ui->m_openglWidget->cycleLUT(-1);
        else
            m_ui->m_openglWidget->cycleLUT(1);
    }
    else if (key == Qt::Key_N) {
        m_force_frame = true;
    }
    else if (key == Qt::Key_Q) {
        m_request_quit = true;
    }
    else if (key == Qt::Key_R) {

        if (m_source != LPSource::None) {
            toggleRecord();
        }
    }
    else if (key == Qt::Key_S) {
        m_ui->m_settings_action->trigger();
    }
}

void LPOpenGLWidget::keyReleaseEvent(QKeyEvent *event) {

    const int key = event->key();

    if (key == Qt::Key_Shift)
        m_shift_pressed = false;
}

void LPOpenGLWidget::wheelEvent(QWheelEvent *event) {

    int new_value = m_settings_ui->m_scale_horizontal_slider->value() + event->angleDelta().y() / 5;
    if (new_value < 0)
        new_value = 0;
    else if (new_value > MAX_SLIDER_VALUE)
        new_value = MAX_SLIDER_VALUE;
    m_settings_ui->m_scale_horizontal_slider->setValue(new_value);
}

void LPOpenGLWidget::setSpeed(double speed) {
    //m_media_player.setPlaybackRate(speed);
    m_video_input.setSpeed(speed);
    printf("New speed: %f\n", speed);
}

void LPOpenGLWidget::cycleLUT(int direction) {

    m_current_lut += direction;
    if (m_current_lut < 0)
        m_current_lut = m_lut_textures.size() - 1;
    else if (m_current_lut >= m_lut_textures.size())
        m_current_lut = 0;

    repaint();
}
