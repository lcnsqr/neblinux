#ifndef FORMHEAT_H
#define FORMHEAT_H

#include <QWidget>

#include "devNano.h"

#include <QCheckBox>
#include <QSpinBox>

namespace Ui {
class FormHeat;
}

class FormHeat : public QWidget
{
    Q_OBJECT

public:
    explicit FormHeat(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormHeat();

    QCheckBox *getPidEnabled();

    QSpinBox* getHeatLoad();

public slots:
    void pidEnabledChange(int state);

    void heatLoadChange();
    void heatLoadSave();

private:
    Ui::FormHeat *ui;
    devNano *dev;
};

#endif // FORMHEAT_H
