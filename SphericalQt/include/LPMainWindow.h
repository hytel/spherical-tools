#ifndef LP_MAIN_WINDOW_HPP
#define LP_MAIN_WINDOW_HPP

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
#include <QMainWindow>
#include "LPSettingsDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LPMainWindow; }
QT_END_NAMESPACE

class LPMainWindow : public QMainWindow {

    Q_OBJECT

public:

    LPMainWindow(QWidget *parent = nullptr);
    ~LPMainWindow();

    // Overrides
    void closeEvent (QCloseEvent *event) override;

    // Actions
    void showSettings(void);
    void scaleChanged(int value);
    void filterChanged(int value);
    void rotateX(int value);
    void rotateY(int value);
    void rotateZ(int value);
    void shiftX(int value);
    void shiftY(int value);
    void gammaChanged(int value);
    void brightnessChanged(int value);
    void saturationChanged(int value);
    void temperatureChanged(int value);
    void vignetteIntensityChanged(int value);
    void vignetteExtentChanged(int value);
    void saveAction(void);
    void changeSpeed(void);
    void changeFPS(void);
    void recordStartAction(void);
    void recordStopAction(void);
    void quitAction(void);

    // Public methods
    int  loop(QApplication *app);

private:

    // Private members
    Ui::LPMainWindow     *m_ui = nullptr;
    LPSettingsDialog     *m_settings_dialog = nullptr;
    Ui::LPSettingsDialog *m_settings_ui = nullptr;
    bool                  m_done = false;
};

#endif // LP_MAIN_WINDOW_HPP
