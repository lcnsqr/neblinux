#ifndef FORMPREFS_H
#define FORMPREFS_H

#include <QWidget>

#include "devNano.h"

#include <QCheckBox>
#include <QSpinBox>

namespace Ui {
class FormPrefs;
}

class FormPrefs : public QWidget
{
    Q_OBJECT

public:
    explicit FormPrefs(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormPrefs();

    QCheckBox *getAutostop();
    QSpinBox *getTempstep();
    QCheckBox *getScreensaver();

public slots:
    void autostopChange(int state);
    void tempstepChange();
    void tempstepSave();
    void screensaverChange(int state);

private:
    Ui::FormPrefs *ui;
    devNano *dev;
};

#endif // FORMPREFS_H
