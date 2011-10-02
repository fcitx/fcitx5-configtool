#ifndef FCITX_SKIN_PAGE_H
#define FCITX_SKIN_PAGE_H

#include <QWidget>

#include "ui_FcitxSkinPage.h"


namespace Fcitx
{
    class Module;

    class FcitxSkinPage : public QWidget
    {
        Q_OBJECT
    public:
        FcitxSkinPage(Module* module, QWidget* parent = 0);
        virtual ~FcitxSkinPage();
    public Q_SLOTS:
        void load();
        void save();
    Q_SIGNALS:
        void changed();
    protected Q_SLOTS:
        void installButtonClicked();
    private:
        class Private;
        Module* m_module;
        Private* d;
        Ui::FcitxSkinPage* m_ui;
    };
    
}

#endif