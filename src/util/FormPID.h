#ifndef FORMPID_H
#define FORMPID_H

#include <QWidget>

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

    void updateScreenData();
    void devDataIn(const struct State& state);
    void reset();

public slots:
    void apply();
    void restore();

    void cPID0Change();
    void cPID1Change();
    void cPID2Change();

private:
    Ui::FormPID *ui;
    devNano *dev;

};

#endif // FORMPID_H
