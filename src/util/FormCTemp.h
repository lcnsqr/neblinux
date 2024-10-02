#ifndef FORMCTEMP_H
#define FORMCTEMP_H

#include <QWidget>
#include <QDoubleSpinBox>

#include "devNano.h"

namespace Ui {
class FormCTemp;
}

class FormCTemp : public QWidget
{
    Q_OBJECT

public:
    explicit FormCTemp(QWidget *parent = nullptr, devNano *d = nullptr);
    ~FormCTemp();

    // Provide access to cTemp coefs
    QDoubleSpinBox* getCTemp0();
    QDoubleSpinBox* getCTemp1();
    QDoubleSpinBox* getCTemp2();
    QDoubleSpinBox* getCTemp3();
    QDoubleSpinBox* getCTemp(int i);

public slots:
    void CTemp0Change();
    void CTemp0Save();
    void CTemp1Change();
    void CTemp1Save();
    void CTemp2Change();
    void CTemp2Save();
    void CTemp3Change();
    void CTemp3Save();

private:
    Ui::FormCTemp *ui;
    devNano *dev;
};

#endif // FORMCTEMP_H
