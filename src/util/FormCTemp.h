#ifndef FORMCTEMP_H
#define FORMCTEMP_H

#include <QWidget>

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

    void updateScreenData();
    void devDataIn(const struct State& state);
    void reset();

    void setCTempAll(const QList<float> &values);

    QList<float> getCTempAll();

    void upload();

public slots:
    void apply();
    void restore();

    void CTemp0Change();
    void CTemp1Change();
    void CTemp2Change();
    void CTemp3Change();


signals:
    void CTempChange();

private:
    Ui::FormCTemp *ui;
    devNano *dev;
};

#endif // FORMCTEMP_H
