#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <KComboBox>
#include <KLocalizedString>

#include <fcitxqtkeyboardproxy.h>
#include <fcitxqtconnection.h>

#include "imconfigdialog.h"
#include "global.h"
#include "configwidget.h"
#include "keyboardlayoutwidget.h"

Fcitx::IMConfigDialog::IMConfigDialog(const QString& imName, const FcitxAddon* addon, QWidget* parent): QDialog(parent)
    ,m_imName(imName)
    ,m_layoutCombobox(0)
    ,m_configPage(0)
{
    QWidget* widget = new QWidget(this);
    QVBoxLayout* l = new QVBoxLayout(this);
    widget->setLayout(l);

    if (!imName.startsWith("fcitx-keyboard") && Global::instance()->keyboardProxy()) {
        QDBusPendingReply< FcitxQtKeyboardLayoutList > layoutList = Global::instance()->keyboardProxy()->GetLayouts();
        layoutList.waitForFinished();

        if (!layoutList.isError()) {
            m_layoutList = layoutList.value();
            m_layoutCombobox = new KComboBox(this);

            QDBusPendingReply< QString, QString > res = Global::instance()->keyboardProxy()->GetLayoutForIM(imName);
            res.waitForFinished();
            QString imLayout = qdbus_cast<QString>(res.argumentAt(0));
            QString imVariant =  qdbus_cast<QString>(res.argumentAt(1));

            QLabel* label;
            label = new QLabel(i18n("<b>Keyboard Layout:</b>"));

            int idx = 1;
            int select = 0;
            if (imName == "default")
                m_layoutCombobox->addItem(i18n("Default"));
            else
                m_layoutCombobox->addItem(i18n("Input Method Default"));

            foreach (const FcitxQtKeyboardLayout& layout, layoutList.value()) {
                if (imLayout == layout.layout() && imVariant == layout.variant())
                    select = idx;
                m_layoutCombobox->addItem(layout.name());
                idx ++;
            }
            m_layoutCombobox->setCurrentIndex(select);

            l->addWidget(label);
            l->addWidget(m_layoutCombobox);
            connect(m_layoutCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(layoutComboBoxChanged()));
#if 0
            m_layoutWidget = new KeyboardLayoutWidget(this);
            m_layoutWidget->setMinimumSize(QSize(400, 200));
            m_layoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            l->addWidget(m_layoutWidget);
#endif
            layoutComboBoxChanged();
        }
    }
    else {
#if 0
        KeyboardLayoutWidget* layoutWidget = new KeyboardLayoutWidget(this);
        layoutWidget->setMinimumSize(QSize(400, 200));
        layoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QString layoutstring = imName.mid(strlen("fcitx-keyboard-"));
        int p = layoutstring.indexOf("-");
        QString layout, variant;
        if (p < 0) {
            layout = layoutstring;
        }
        else {
            layout = layoutstring.mid(0, p);
            variant = layoutstring.mid(p + 1);
        }
        layoutWidget->setKeyboardLayout(layout, variant);
        l->addWidget(layoutWidget);
#endif
    }

    FcitxConfigFileDesc* cfdesc = NULL;

    if (addon) {
        cfdesc = Global::instance()->GetConfigDesc(QString::fromUtf8(addon->name).append(".desc"));

        if (cfdesc ||  strlen(addon->subconfig) != 0) {
            if (m_layoutCombobox) {
                QLabel* label = new QLabel(i18n("<b>Input Method Setting:</b>"));
                label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
                l->addWidget(label);
            }
            m_configPage = new ConfigWidget(
                cfdesc,
                QString::fromUtf8("conf"),
                QString::fromUtf8(addon->name).append(".config") ,
                QString::fromUtf8(addon->subconfig),
                QString::fromUtf8(addon->name),
                this
            );
            l->addWidget(m_configPage);
        }
    }
    setWindowIcon(QIcon::fromTheme("fcitx"));
    auto dialogLayout = new QVBoxLayout;
    setLayout(dialogLayout);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    dialogLayout->addWidget(widget);
    dialogLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::clicked, this, [this, buttonBox](QAbstractButton* button) {
        this->onButtonClicked(buttonBox->standardButton(button));
    });
    connect(buttonBox, &QDialogButtonBox::accepted, this, &Fcitx::IMConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &Fcitx::IMConfigDialog::reject);
}

void Fcitx::IMConfigDialog::onButtonClicked(QDialogButtonBox::StandardButton code)
{
    if (m_layoutCombobox && Global::instance()->keyboardProxy()) {
        if (code == QDialogButtonBox::Ok) {
            int idx = m_layoutCombobox->currentIndex();
            if (idx == 0)
                Global::instance()->keyboardProxy()->SetLayoutForIM(m_imName, "", "");
            else
                Global::instance()->keyboardProxy()->SetLayoutForIM(m_imName, m_layoutList.at(idx - 1).layout(), m_layoutList.at(idx - 1).variant());
        }
        else if (code == QDialogButtonBox::RestoreDefaults)
            m_layoutCombobox->setCurrentIndex(0);
    }

    if (m_configPage)
        m_configPage->buttonClicked(code);
}

void Fcitx::IMConfigDialog::layoutComboBoxChanged()
{
#if 0
    if (!m_layoutCombobox || !m_layoutWidget)
#endif
        return;
#if 0
    int idx = m_layoutCombobox->currentIndex();
    if (idx != 0) {
        m_layoutWidget->setKeyboardLayout(m_layoutList.at(idx - 1).layout(), m_layoutList.at(idx - 1).variant());
        m_layoutWidget->show();
    }
    else
        m_layoutWidget->hide();
#endif
}
