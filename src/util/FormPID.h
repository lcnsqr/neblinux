#ifndef FORMPID_H
#define FORMPID_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>

#include "devNano.h"

namespace Ui {
class FormPID;
}

class FormPID : public QWidget
{
    Q_OBJECT

public:
    explicit FormPID(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormPID();

    // Provide access to the target field
    QSpinBox* getTarget();

    // Provide access to PID coefs
    QDoubleSpinBox* getCPID(int i);

    // Access to action buttons
    QPushButton *getcPIDrestore();
    QPushButton *getcPIDapply();

public slots:
    void apply();
    void restore();

    void targetChange();
    void cPID0Change();
    void cPID1Change();
    void cPID2Change();

private:
    Ui::FormPID *ui;
    devNano *dev;

};

#endif // FORMPID_H
