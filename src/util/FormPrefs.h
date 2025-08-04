#ifndef FORMPREFS_H
#define FORMPREFS_H

#include <QWidget>

#include "devNano.h"


namespace Ui {
class FormPrefs;
}

class FormPrefs : public QWidget
{
    Q_OBJECT

public:
    explicit FormPrefs(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormPrefs();

    void updateScreenData();
    void devDataIn(const struct State& state);
    void reset();

public slots:
    void apply();
    void restore();

    void autostopChange();
    void tempstepChange();
    void targetChange();
    void screensaverChange();

private:
    Ui::FormPrefs *ui;
    devNano *dev;
};

#endif // FORMPREFS_H
