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

#include "LPSettingsDialog.h"
#include "ui_LPSettingsDialog.h"
#include "LPOpenGLWidget.h"

LPSettingsDialog::LPSettingsDialog(LPOpenGLWidget *opengl_widget) :
    m_ui(new Ui::LPSettingsDialog), m_openglWidget(opengl_widget) {
    m_ui->setupUi(this);

    connect(m_ui->m_reset_push_button, &QPushButton::clicked, this, &LPSettingsDialog::resetSettings);
    connect(m_ui->m_close_push_button, &QPushButton::clicked, this, &LPSettingsDialog::closeAction);
}

void LPSettingsDialog::closeAction(void) {
    hide();
}

void LPSettingsDialog::resetSettings(void) {
    m_openglWidget->resetSettings();
}
