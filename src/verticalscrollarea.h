#ifndef VERTICALSCROLLAREA_H
#define VERTICALSCROLLAREA_H

#include <QScrollArea>
namespace Fcitx
{
class VerticalScrollArea : public QScrollArea {
    Q_OBJECT
public:
    explicit VerticalScrollArea(QWidget* parent = 0);
    void setWidget(QWidget *widget);
protected:
    virtual bool eventFilter(QObject* o, QEvent* e);
};
}

#endif // VERTICALSCROLLAREA_H