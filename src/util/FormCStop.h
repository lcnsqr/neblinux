#ifndef FORMCSTOP_H
#define FORMCSTOP_H

#include <QWidget>
#include <QDoubleSpinBox>

#include "devNano.h"

namespace Ui {
class FormCStop;
}

class FormCStop : public QWidget
{
    Q_OBJECT

public:
    explicit FormCStop(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormCStop();

    // Provide access to cStop coefs
    QDoubleSpinBox* getCStop0();
    QDoubleSpinBox* getCStop1();

public slots:
    void cStop0Change();
    void cStop0Save();
    void cStop1Change();
    void cStop1Save();


private:
    Ui::FormCStop *ui;
    devNano *dev;
};

#endif // FORMCSTOP_H
