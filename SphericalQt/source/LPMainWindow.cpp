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
#include <QtGui/QActionGroup>

// Qt Spherical includes
#include "LPMainWindow.h"
#include "LPRecordAnimation.h"

// Static variables
static LPRecordAnimation record_animation;

LPMainWindow::LPMainWindow(QWidget *parent)
    : QMainWindow(parent), 
      m_ui(new Ui::LPMainWindow), 
      m_done(false)
{
    m_ui->setupUi(this);

    m_settings_dialog = new LPSettingsDialog(m_ui->m_openglWidget);
    m_settings_ui = m_settings_dialog->getUI();

    record_animation.setUI(m_ui, m_settings_ui);

    connect(m_ui->m_settings_action, &QAction::triggered, this, &LPMainWindow::showSettings);

    connect(m_settings_ui->m_scale_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::scaleChanged);

    connect(m_settings_ui->m_rotate_x_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::rotateY);
    connect(m_settings_ui->m_rotate_y_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::rotateX);
    connect(m_settings_ui->m_rotate_z_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::rotateZ);

    connect(m_settings_ui->m_shift_x_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::shiftX);
    connect(m_settings_ui->m_shift_y_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::shiftY);

    connect(m_settings_ui->m_gamma_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::gammaChanged);
    connect(m_settings_ui->m_brightness_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::brightnessChanged);
    connect(m_settings_ui->m_saturation_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::saturationChanged);
    connect(m_settings_ui->m_temperature_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::temperatureChanged);

    connect(m_settings_ui->m_vignette_intensity_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::vignetteIntensityChanged);
    connect(m_settings_ui->m_vignette_extent_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::vignetteExtentChanged);

    connect (m_settings_ui->m_filter_horizontal_slider, &QSlider::valueChanged, this, &LPMainWindow::filterChanged);

    connect(m_ui->m_save_action, &QAction::triggered, this, &LPMainWindow::saveAction);
    connect(m_ui->m_quit_action, &QAction::triggered, this, &LPMainWindow::quitAction);

    connect(m_ui->m_record_start, &QAction::triggered, this, &LPMainWindow::recordStartAction);
    connect(m_ui->m_record_stop, &QAction::triggered, this, &LPMainWindow::recordStopAction);

    connect(m_ui->m_speed_20, &QAction::triggered, this, &LPMainWindow::changeSpeed);
    connect(m_ui->m_speed_10, &QAction::triggered, this, &LPMainWindow::changeSpeed);
    connect(m_ui->m_speed_05, &QAction::triggered, this, &LPMainWindow::changeSpeed);
    connect(m_ui->m_speed_25, &QAction::triggered, this, &LPMainWindow::changeSpeed);
    connect(m_ui->m_speed_01, &QAction::triggered, this, &LPMainWindow::changeSpeed);

    connect(m_ui->m_fps_10, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_15, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_20, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_24, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_2997, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_30, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_60, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_80, &QAction::triggered, this, &LPMainWindow::changeFPS);
    connect(m_ui->m_fps_100, &QAction::triggered, this, &LPMainWindow::changeFPS);

    connect(m_ui->m_azimuth_loop_action, &QAction::triggered, &record_animation, &LPRecordAnimation::recordAzimuthLoop);

    m_ui->m_openglWidget->setUI(this, m_ui, m_settings_ui);

    // Setup action groups
    QActionGroup *fps_action_group = new QActionGroup(this);
    fps_action_group->addAction(m_ui->m_fps_10);
    fps_action_group->addAction(m_ui->m_fps_15);
    fps_action_group->addAction(m_ui->m_fps_20);
    fps_action_group->addAction(m_ui->m_fps_24);
    fps_action_group->addAction(m_ui->m_fps_2997);
    fps_action_group->addAction(m_ui->m_fps_30);
    fps_action_group->addAction(m_ui->m_fps_60);
    fps_action_group->addAction(m_ui->m_fps_80);
    fps_action_group->addAction(m_ui->m_fps_100);

    m_ui->m_openglWidget->resetSettings();

    // Initialize projection parametes
    scaleChanged(m_settings_ui->m_scale_horizontal_slider->value());
    rotateX(m_settings_ui->m_rotate_y_horizontal_slider->value());
    rotateY(m_settings_ui->m_rotate_x_horizontal_slider->value());
    rotateZ(m_settings_ui->m_rotate_z_horizontal_slider->value());
    shiftX(m_settings_ui->m_shift_x_horizontal_slider->value());
    shiftY(m_settings_ui->m_shift_y_horizontal_slider->value());
    gammaChanged(m_settings_ui->m_gamma_horizontal_slider->value());
    brightnessChanged(m_settings_ui->m_brightness_horizontal_slider->value());
    vignetteIntensityChanged(m_settings_ui->m_vignette_intensity_horizontal_slider->value());
    vignetteExtentChanged(m_settings_ui->m_vignette_extent_horizontal_slider->value());

    repaint();
}

void LPMainWindow::showSettings(void) {
    m_settings_dialog->show();
}

void LPMainWindow::saveAction(void) {
    m_ui->m_openglWidget->save();
}

void LPMainWindow::quitAction(void) {
    m_done = true;
}

LPMainWindow::~LPMainWindow()
{
    delete m_ui;
}

void LPMainWindow::scaleChanged(int value) { 
    double max_val = MAX_SLIDER_VALUE;
    float scale = (((float)value / max_val) * 50.0f) + 0.4f;
    m_ui->m_openglWidget->setScale(scale);
}

void LPMainWindow::filterChanged(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float filter_strength = (float)value / max_val;
    m_ui->m_openglWidget->setFilterStrength(filter_strength);
}

void LPMainWindow::rotateX(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float angle = ((float)value / max_val) * 360.0;
    m_ui->m_openglWidget->setRotateX(angle);
}

void LPMainWindow::rotateY(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float angle = ((float)value / max_val) * 360.0;
    m_ui->m_openglWidget->setRotateY(angle);
}

void LPMainWindow::rotateZ(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float angle = ((float)value / max_val) * 360.0;
    m_ui->m_openglWidget->setRotateZ(angle);
}

void LPMainWindow::shiftX(int value) {
    double max_val_half = (double)MAX_SLIDER_VALUE / 2.0;
    float shift = ((value-max_val_half) / max_val_half) * 1.5;
    m_ui->m_openglWidget->setShiftX(-shift);
}

void LPMainWindow::shiftY(int value) {
    double max_val_half = (double)MAX_SLIDER_VALUE / 2.0;
    float shift = ((value-max_val_half) / max_val_half) * 1.5;
    m_ui->m_openglWidget->setShiftY(-shift);
}

void LPMainWindow::gammaChanged(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float gamma_value = ((7.0 * (float)value) / max_val) + 0.3f;
    m_ui->m_openglWidget->setGamma(gamma_value);
}

void LPMainWindow::brightnessChanged(int value) {
    double max_val_half = (double)MAX_SLIDER_VALUE / 2.0;
    float brightness_value = ((float)(value - max_val_half)) / max_val_half;
    m_ui->m_openglWidget->setBrightness(brightness_value);
}

void LPMainWindow::saturationChanged(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float saturation_value = (float)value / max_val;
    m_ui->m_openglWidget->setSaturation(saturation_value);
}

void LPMainWindow::temperatureChanged(int value) {
    float temperature_value = (float)value;
    m_ui->m_openglWidget->setTemperature(temperature_value);
}

void LPMainWindow::vignetteIntensityChanged(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float vignette_intensity = (((float)value / max_val) * 20.0f) + 10.0f;
    m_ui->m_openglWidget->setVignetteIntensity(vignette_intensity);
}

void LPMainWindow::vignetteExtentChanged(int value) {
    double max_val = MAX_SLIDER_VALUE;
    float vignette_extent = 3.0 * (float)value / max_val;
    m_ui->m_openglWidget->setVignetteExtent(vignette_extent);
}

void LPMainWindow::recordStartAction(void){
    //m_ui->m_openglWidget->record(true);
}

void LPMainWindow::recordStopAction(void) {
    //m_ui->m_openglWidget->record(false);
}

void LPMainWindow::changeSpeed(void) {
    QAction *the_sender = static_cast<QAction *>(sender());
    QString text = the_sender->text();
    if (text == "2.0")
        m_ui->m_openglWidget->setSpeed(2.0);
    else  if (text == "1.0")
        m_ui->m_openglWidget->setSpeed(1.0);
    else  if (text == "0.5")
        m_ui->m_openglWidget->setSpeed(0.5);
    else  if (text == "0.25")
        m_ui->m_openglWidget->setSpeed(0.25);
    else  if (text == "0.1")
        m_ui->m_openglWidget->setSpeed(0.1);
    else
        printf("Invalid speed!\n");
}

void LPMainWindow::changeFPS(void) {

    //QAction *the_sender = static_cast<QAction *>(sender());
    //QString text = the_sender->text();
    //if (text == "100")
    //    m_ui->m_openglWidget->setFPS(100);
    //else  if (text == "80")
    //    m_ui->m_openglWidget->setFPS(80);
    //else  if (text == "60")
    //    m_ui->m_openglWidget->setFPS(60);
    //else  if (text == "30")
    //    m_ui->m_openglWidget->setFPS(30);
    //else  if (text == "29.97")
     //   m_ui->m_openglWidget->setFPS(29.97);
    //else  if (text == "24")
    //    m_ui->m_openglWidget->setFPS(24);
    //else  if (text == "15")
    //    m_ui->m_openglWidget->setFPS(15);
    //else  if (text == "10")
    //    m_ui->m_openglWidget->setFPS(10);
    //else
    //    printf("Invalid FPS!\n");
}

void LPMainWindow::closeEvent (QCloseEvent *) {
    m_done = true;
}

int LPMainWindow::loop(QApplication *app) {

    m_ui->m_openglWidget->setApp(app);

    while (!m_done) {

        record_animation.update();

        if (m_ui->m_openglWidget->getRequestQuit())
            m_done = true;

        m_ui->m_openglWidget->update();

        const QString &filter_string = m_ui->m_openglWidget->getCurrentFilterString();
        QString mode_string = "Idle";
        if (m_ui->m_openglWidget->isRecording())
            mode_string  = "Recording";
        else if (m_ui->m_openglWidget->getSource() == LPSource::Image)
            mode_string = "Image";
        else if (m_ui->m_openglWidget->getSource() == LPSource::Video) {
            if (m_ui->m_openglWidget->isPaused())
                mode_string = "Video Paused";
            else
                mode_string = "Video Playing";
        }

        m_ui->m_status_bar->showMessage(QString("Mode: %1  |  Filter: %2").arg(mode_string).arg(filter_string));

        app->processEvents();

        m_ui->m_openglWidget->frameDone();
    }

    // Exit this Qt application
    app->exit();

    exit(0);

    return 1;
}
