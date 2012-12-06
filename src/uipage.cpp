#include "uipage.h"
#include "module.h"
#include "configwidget.h"
#include "configdescmanager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <KLocalizedString>
#include <fcitx-qt/fcitxqtinputmethodproxy.h>

Fcitx::UIPage::UIPage(Fcitx::Module* parent) : QWidget(parent)
    ,m_module(parent)
    ,m_proxy (m_module->inputMethodProxy())
    ,m_layout(new QVBoxLayout(this))
    ,m_label(new QLabel(i18n("Cannot load currently used user interface info"), this))
    ,m_widget(0)
{
    setLayout(m_layout);
    m_layout->addWidget(m_label);
    QDBusPendingReply< QString > reply = m_proxy->GetCurrentUI();
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(getUIFinished(QDBusPendingCallWatcher*)));
}

void Fcitx::UIPage::getUIFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QString> reply(*watcher);
    if (!reply.isValid())
        return;
    QString name = reply.value();
    FcitxAddon* addon = m_module->findAddonByName(name);
    if (addon) {
        FcitxConfigFileDesc* cfdesc = ConfigDescManager::instance()->GetConfigDesc(QString::fromUtf8(addon->name).append(".desc"));
        bool configurable = (bool)(cfdesc != NULL || strlen(addon->subconfig) != 0);
        if (configurable) {
            m_label->hide();
            m_widget = new ConfigWidget(addon, this);
            m_layout->addWidget(m_widget);
            connect(m_widget, SIGNAL(changed()), this, SIGNAL(changed()));
        }
        else {
            m_label->setText(i18n("No configuration options for %1.").arg(QString::fromUtf8(addon->generalname)));
        }
    }
}

void Fcitx::UIPage::load()
{
    if (m_widget)
        m_widget->load();
}


void Fcitx::UIPage::save()
{
    if (m_widget)
        m_widget->buttonClicked(KDialog::Ok);
}

Fcitx::UIPage::~UIPage()
{

}
