#ifndef FORMCSTOP_H
#define FORMCSTOP_H

#include <QWidget>

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

    void updateScreenData();
    void devDataIn(const struct State& state);
    void reset();

public slots:
    void apply();
    void restore();

    void cStop0Change();
    void cStop1Change();

private:
    Ui::FormCStop *ui;
    devNano *dev;
};

#endif // FORMCSTOP_H
