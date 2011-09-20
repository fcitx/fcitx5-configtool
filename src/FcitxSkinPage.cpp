#include "FcitxSkinPage.h"
#include <KNS3/DownloadDialog>
#include <KDebug>

namespace Fcitx
{
    FcitxSkinPage::FcitxSkinPage(QWidget* parent):
        QWidget(parent),
        m_ui(new Ui::FcitxSkinPage)
        
    {
        m_ui->setupUi(this);
        m_ui->installSkinButton->setIcon(KIcon("get-hot-new-stuff"));
        connect(m_ui->installSkinButton, SIGNAL(clicked()), this, SLOT(installButtonClicked()));
    }
    
    FcitxSkinPage::~FcitxSkinPage()
    {
        delete m_ui;
    }
    
    void FcitxSkinPage::installButtonClicked()
    {  
        KNS3::DownloadDialog dialog("fcitx-skin.knsrc");
        dialog.exec();
        foreach (const KNS3::Entry& e, dialog.changedEntries()) {
            kDebug() << "Changed Entry: " << e.name();
        }
    }

}