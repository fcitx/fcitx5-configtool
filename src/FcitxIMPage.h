#ifndef FCITX_IM_PAGE_H
#define FCITX_IM_PAGE_H
#include <QString>
#include <QWidget>
#include <QDBusConnection>
#include "org.fcitx.Fcitx.InputMethod.h"

namespace Ui
{
    class FcitxIMPage;
}

namespace Fcitx
{    
    class FcitxIMPage : public QWidget
    {
        Q_OBJECT
    public:
        FcitxIMPage(QWidget* parent = 0);
        virtual ~FcitxIMPage();
    Q_SIGNALS:
        void changed();
    public Q_SLOTS:
        void save();
        void load();
    private:
        Ui::FcitxIMPage* m_ui;
        
        class Private;
        Private* d;
    };
}

#endif