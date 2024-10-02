#ifndef FORMPID_H
#define FORMPID_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>

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

public slots:
    void targetChange();
    void targetSave();
    void cPID0Change();
    void cPID0Save();
    void cPID1Change();
    void cPID1Save();
    void cPID2Change();
    void cPID2Save();

private:
    Ui::FormPID *ui;
    devNano *dev;
};

#endif // FORMPID_H
