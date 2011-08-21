#ifndef FCITXCONFIGPAGE_H
#define FCITXCONFIGPAGE_H

#include <QWidget>
#include <ui_FcitxConfigPage.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>

class QStandardItemModel;
struct _ConfigFileDesc;

class QTabWidget;

namespace Fcitx
{

class FcitxSubConfigParser;


    class FcitxConfigPage : public QWidget
    {
        Q_OBJECT

    public:
        FcitxConfigPage ( QWidget* parent, struct _ConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subConfig = QString());
        virtual ~FcitxConfigPage();

    Q_SIGNALS:
        void changed();

    public Q_SLOTS:
        void buttonClicked ( KDialog::ButtonCode );
    private:
        void setupConfigUi();
        void setupSubConfigUi();

        struct _ConfigFileDesc* m_cfdesc;
        QString m_prefix;
        QString m_name;
        QTabWidget* m_tabWidget;
        Ui::FcitxConfigPage* m_ui;
        GenericConfig gconfig;
        FcitxSubConfigParser* m_parser;
    };

}

#endif
