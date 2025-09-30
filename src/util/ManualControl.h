#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <QWidget>
#include "devNano.h"

namespace Ui {
class ManualControl;
}

class ManualControl : public QWidget
{
    Q_OBJECT

public:
    explicit ManualControl(QWidget *parent = nullptr, devNano *d = nullptr);
    ~ManualControl();

    void updateScreenData();
    void devDataIn(const struct State& state);
    void reset();

public slots:
    void apply();
    void restore();

    void fanChange();
    void PIDChange();
    void loadChange();

private:
    Ui::ManualControl *ui;
    devNano *dev;
};

#endif // MANUALCONTROL_H
