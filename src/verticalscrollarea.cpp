#include <QVBoxLayout>
#include <QEvent>
#include <QScrollBar>

#include "verticalscrollarea.h"

namespace Fcitx {
VerticalScrollArea::VerticalScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void VerticalScrollArea::setWidget(QWidget* widget)
{
    QScrollArea::setWidget(widget);
    widget->installEventFilter(this);
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e)
{
    if(o == widget() && e->type() == QEvent::Resize)
        setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());

    return false;
}

}