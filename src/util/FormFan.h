#ifndef FORMFAN_H
#define FORMFAN_H

#include <QWidget>

#include "devNano.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

namespace Ui {
class FormFan;
}

class FormFan : public QWidget
{
    Q_OBJECT

public:
    explicit FormFan(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormFan();

    QCheckBox *getFanControl();

    QSpinBox* getFanLoad();

    QLabel *getElapsed();

public slots:
    void fanControlChange(int state);

    void fanLoadChange();
    void fanLoadSave();

private:
    Ui::FormFan *ui;
    devNano *dev;
};

#endif // FORMFAN_H
