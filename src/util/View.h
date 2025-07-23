#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QAction>
#include <QShowEvent>
#include <QCloseEvent>
#include <QSettings>

class View : public QWidget
{
    Q_OBJECT
public:
    explicit View(QWidget *parent = nullptr, QAction *menu = nullptr);

    QVBoxLayout *layout;

    void store();
    void restore();

private:
    QAction *menu;
    QSettings settings;

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
};

#endif // VIEW_H
