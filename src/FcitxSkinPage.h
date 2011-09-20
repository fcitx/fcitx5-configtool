#include <QWidget>

#include "ui_FcitxSkinPage.h"

namespace Fcitx
{
    class FcitxSkinPage : public QWidget
    {
        Q_OBJECT
    public:
        FcitxSkinPage(QWidget* parent = 0);
        virtual ~FcitxSkinPage();
    protected Q_SLOTS:
        void installButtonClicked();
    private:
        Ui::FcitxSkinPage* m_ui;
    };
    
}