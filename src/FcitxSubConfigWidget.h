
#ifndef FCITXCONFIGPAGE_P_H
#define FCITXCONFIGPAGE_P_H

#include <QWidget>

class QListView;
namespace Fcitx
{

    class FcitxConfigFileItemModel;
    class FcitxSubConfig;

    class FcitxSubConfigWidget : public QWidget
    {
        Q_OBJECT
    public:
        FcitxSubConfigWidget ( FcitxSubConfig* subconfig, QWidget* parent = 0 );

    private slots:
        void OpenSubConfig();
        void OpenNativeFile();

    private:
        FcitxSubConfig* m_subConfig;
        FcitxConfigFileItemModel* m_model;
        QListView* m_listView;
    };

}

#endif