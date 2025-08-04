#include "View.h"
#include <QTimer>

View::View(QWidget *parent, QAction *m):
    QWidget{parent},
    menu(m)
{
    layout = new QVBoxLayout(this);

    setWindowTitle(menu->toolTip());
}

void View::store()
{
    settings.beginGroup("Visible_views");
    settings.setValue(menu->objectName(), isVisible());
    settings.endGroup();

    if ( isVisible() ){
        settings.beginGroup("Geometry");
        settings.setValue(menu->objectName(), saveGeometry());
        settings.endGroup();
    }
}

void View::restore()
{
    settings.beginGroup("Geometry");
    if (settings.contains(menu->objectName()))
        restoreGeometry(settings.value(menu->objectName()).toByteArray());
    else
        resize(400, -1);
    settings.endGroup();

    settings.beginGroup("Visible_views");
    bool showView = settings.value(menu->objectName(), false).toBool();
    settings.endGroup();

    if ( showView ){
        if ( menu != nullptr ) menu->setChecked(true);
        show();
    }

}

void View::closeEvent(QCloseEvent *event)
{
    if ( isVisible() ){
        settings.beginGroup("Geometry");
        settings.setValue(menu->objectName(), saveGeometry());
        settings.endGroup();
    }

    if ( menu != nullptr ) menu->setChecked(false);

    event->accept();
}

void View::showEvent(QShowEvent *event)
{
    settings.beginGroup("Visible_views");
    settings.setValue(menu->objectName(), true);
    settings.endGroup();

    event->accept();
}
