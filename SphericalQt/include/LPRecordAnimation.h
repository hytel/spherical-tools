#ifndef LP_RECORD_ANIMATION_HPP
#define LP_RECORD_ANIMATION_HPP

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

// Qt UI includes
#include "ui_LPMainWindow.h"
#include "ui_LPSettingsDialog.h"

class LPRecordAnimation : public QObject {

public:

    enum AnimationState {
    Idle,
    Azimuth
    };

    LPRecordAnimation() : m_ui(nullptr), m_state(Idle), m_first_pos(0), m_current_pos(0) {

    }

    void setUI(Ui::LPMainWindow *ui, Ui::LPSettingsDialog *settings_ui) {
        m_ui = ui;
        m_settings_ui = settings_ui;
    }

    void recordAzimuthLoop(void) {

        // Stop azimuth loop if running
        if (m_state == Azimuth) {
            m_state = Idle;
            return;
        }

        m_state = Azimuth;
        m_first_pos = m_settings_ui->m_rotate_x_horizontal_slider->value();
        m_current_pos = m_first_pos;

        m_ui->m_openglWidget->beginWrite();
    }

    void update(void) {

        if (m_state == Idle)
            return;

        // Process any new frames
        m_ui->m_openglWidget->update();

        // Record frame, move to next
        //m_recorder.writeFrame(m_ui->m_openglWidget->getLastFrame());

        m_current_pos = (m_current_pos + 4) % MAX_SLIDER_VALUE;

        //if (m_current_pos == m_first_pos) {
        //    m_recorder.endRecording();
        //    m_state = Idle;
        //}

        m_settings_ui->m_rotate_x_horizontal_slider->setValue(m_current_pos);
    }

private:

    // Private members
    Ui::LPMainWindow     *m_ui = nullptr;
    Ui::LPSettingsDialog *m_settings_ui = nullptr;
    AnimationState        m_state;
    int                   m_first_pos = 0;
    int                   m_current_pos = 0;
};

#endif // LP_RECORD_ANIMATION_HPP
