/***************************************************************************
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

// Qt
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QDebug>
#include <QStandardItemModel>
#include <QListView>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>

// KDE
#include <KColorButton>
#include <KFontComboBox>
#include <KComboBox>
#include <KKeySequenceWidget>
#include <KLineEdit>
#include <KRun>
#include <KPushButton>
#include <KMessageBox>
#include <KUrlRequester>

// system
#include <libintl.h>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>

// self
#include "config.h"
#include "FcitxConfigPage.h"
#include "FcitxSubConfigParser.h"
#include "FcitxSubConfigWidget.h"
#include "qtkeytrans.h"
#include "ConfigDescManager.h"

#define RoundColor(c) ((c)>=0?((c)<=255?c:255):0)

namespace Fcitx
{
static bool KeySequenceToHotkey(const QKeySequence& keyseq, FcitxHotkey* hotkey);
static QKeySequence HotkeyToKeySequence(FcitxHotkey* hotkey);

static
void SyncFilterFunc(FcitxGenericConfig* gconfig, FcitxConfigGroup *group, FcitxConfigOption *option, void *value, FcitxConfigSync sync, void *arg);

FcitxConfigPage::FcitxConfigPage(QWidget* parent, _FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig) :
    QWidget(parent), m_cfdesc(cfdesc), m_prefix(prefix), m_name(name), m_ui(new Ui::FcitxConfigPage), m_parser(NULL)
{
    m_parser = new FcitxSubConfigParser(subconfig, this);
    gconfig.configFile = NULL;
    m_ui->setupUi(this);
    setupConfigUi();
    setupSubConfigUi();
}

FcitxConfigPage::~FcitxConfigPage()
{
    FcitxConfigFreeConfigFile(gconfig.configFile);
    delete m_ui;
}

void FcitxConfigPage::buttonClicked(KDialog::ButtonCode code)
{
    if (!m_cfdesc)
        return;

    if (code == KDialog::Default) {
        FcitxConfigResetConfigToDefaultValue(&this->gconfig);
        FcitxConfigBindSync(&this->gconfig);
    } else if (code == KDialog::Ok) {
        FILE* fp = FcitxXDGGetFileUserWithPrefix(m_prefix.toLocal8Bit().data(), m_name.toLocal8Bit().data(), "w", NULL);

        if (fp) {
            FcitxConfigSaveConfigFileFp(fp, &gconfig, m_cfdesc);
            fclose(fp);
        }

        const char* reload_config = "kcm_fcitx_reload_config";
        KMessageBox::information(this,
                                 i18n("Not all configuration can be reloaded immediately."),
                                 i18n("Attention"),
                                 reload_config
                                );

        QDBusInterface fcitx(QString("org.fcitx.Fcitx-%1").arg(fcitx_utils_get_display_number()), "/inputmethod", "org.fcitx.Fcitx.InputMethod");
        QDBusReply<void> reply;
        if (fcitx.isValid()) {
            reply = fcitx.call("ReloadConfig");
        }
        if (!reply.isValid()) {
            QStringList commandAndParameters;
            commandAndParameters << "-r";
            QProcess process;
            process.startDetached(FCITX4_EXEC_PREFIX "/bin/fcitx-remote", commandAndParameters);
        }
    }
}

void FcitxConfigPage::load()
{
    if (m_cfdesc) {
        FILE *fp;
        fp = FcitxXDGGetFileWithPrefix(m_prefix.toLocal8Bit().data(), m_name.toLocal8Bit().data(), "r", NULL);
        if (!fp)
            return;

        gconfig.configFile = FcitxConfigParseIniFp(fp, gconfig.configFile);
        FcitxConfigBindSync(&gconfig);
    }
}

void FcitxConfigPage::setupConfigUi()
{
    if (m_cfdesc) {
        bindtextdomain(m_cfdesc->domain, LOCALEDIR);
        bind_textdomain_codeset(m_cfdesc->domain, "UTF-8");

        FILE *fp;
        fp = FcitxXDGGetFileWithPrefix(m_prefix.toLocal8Bit().data(), m_name.toLocal8Bit().data(), "r", NULL);
        FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, m_cfdesc);

        if (fp)
            fclose(fp);

        if (!cfile)
            return;

        gconfig.configFile = cfile;

        FcitxConfigGroupDesc *cgdesc = NULL;

        FcitxConfigOptionDesc *codesc = NULL;

        for (cgdesc = m_cfdesc->groupsDesc;
                cgdesc != NULL;
                cgdesc = (FcitxConfigGroupDesc*) cgdesc->hh.next) {
            codesc = cgdesc->optionsDesc;

            if (codesc == NULL)
                continue;

            QWidget* main = new QWidget(this);

            QVBoxLayout* mainLayout = new QVBoxLayout;

            main->setLayout(mainLayout);

            QScrollArea *scrollarea = new QScrollArea;

            scrollarea->setFrameStyle(QFrame::NoFrame);

            scrollarea->setWidgetResizable(true);

            QWidget* form = new QWidget;

            QFormLayout* formLayout = new QFormLayout;

            scrollarea->setWidget(form);

            form->setLayout(formLayout);

            int i = 0;

            for (; codesc != NULL;
                    codesc = (FcitxConfigOptionDesc*) codesc->hh.next, i++) {
                QString s;

                if (codesc->desc && strlen(codesc->desc) != 0)
                    s = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->desc));
                else
                    s = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->optionName));

                QWidget *inputWidget = NULL;

                void *argument = NULL;

                switch (codesc->type) {

                case T_Integer: {
                    QSpinBox* spinbox = new QSpinBox(this);
                    spinbox->setMaximum(10000);
                    spinbox->setMinimum(-10000);
                    inputWidget = spinbox;
                    argument = inputWidget;
                    connect(spinbox, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
                    break;
                }

                case T_Color: {
                    KColorButton* colorButton = new KColorButton(this);
                    inputWidget = colorButton;
                    argument = inputWidget;
                    connect(colorButton, SIGNAL(changed(QColor)), this, SIGNAL(changed()));
                }

                break;

                case T_Boolean: {
                    QCheckBox* checkBox = new QCheckBox(this);
                    inputWidget = checkBox;
                    argument = inputWidget;

                    connect(checkBox, SIGNAL(clicked(bool)), this, SIGNAL(changed()));
                    break;
                }

                case T_Font: {
                    KFontComboBox* fontComboBox = new KFontComboBox(this);
                    inputWidget = fontComboBox;
                    argument = inputWidget;
                    connect(fontComboBox, SIGNAL(currentFontChanged(QFont)), this, SIGNAL(changed()));
                }

                break;

                case T_Enum: {
                    int i;
                    FcitxConfigEnum *e = &codesc->configEnum;
                    KComboBox* combobox = new KComboBox(this);
                    inputWidget = combobox;

                    for (i = 0; i < e->enumCount; i ++) {
                        combobox->addItem(QString::fromUtf8(dgettext(m_cfdesc->domain, e->enumDesc[i])));
                    }

                    argument = inputWidget;

                    connect(combobox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
                }

                break;

                case T_Hotkey: {
                    KKeySequenceWidget* keyseq1 = new KKeySequenceWidget();
                    KKeySequenceWidget* keyseq2 = new KKeySequenceWidget();
                    QHBoxLayout* hbox = new QHBoxLayout();
                    hbox->setMargin(0);
                    QWidget* widget = new QWidget(this);
                    keyseq1->setMultiKeyShortcutsAllowed(false);
                    keyseq1->setModifierlessAllowed(true);
                    keyseq2->setMultiKeyShortcutsAllowed(false);
                    keyseq2->setModifierlessAllowed(true);
                    hbox->addWidget(keyseq1);
                    hbox->addWidget(keyseq2);
                    widget->setLayout(hbox);

                    inputWidget = widget;
                    argument = hbox;
                    connect(keyseq1, SIGNAL(keySequenceChanged(QKeySequence)), this, SIGNAL(changed()));
                    connect(keyseq2, SIGNAL(keySequenceChanged(QKeySequence)), this, SIGNAL(changed()));
                }

                break;

                case T_File: {
                    KUrlRequester* requester = new KUrlRequester(this);
                    inputWidget = requester;
                    argument = inputWidget;

                    connect(requester, SIGNAL(urlSelected(KUrl)), this, SIGNAL(changed()));
                }
                break;

                case T_Char:

                case T_String: {
                    KLineEdit* lineEdit = new KLineEdit(this);
                    inputWidget = lineEdit;
                    argument = inputWidget;

                    connect(lineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
                }

                break;

                case T_I18NString:
                    inputWidget = NULL;
                    argument = NULL;
                    break;
                }

                if (inputWidget) {
                    QLabel* label = new QLabel(s, this);
                    formLayout->addRow(label, inputWidget);
                    FcitxConfigBindValue(cfile, cgdesc->groupName, codesc->optionName, NULL, SyncFilterFunc, argument);
                }
            }

            scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            scrollarea->setMinimumWidth(form->sizeHint().width() + 20);
            mainLayout->addWidget(scrollarea);

            m_ui->tabWidget->addTab(main, QString::fromUtf8(dgettext(m_cfdesc->domain, cgdesc->groupName)));
        }

        FcitxConfigBindSync(&gconfig);
    }

}

KDialog* FcitxConfigPage::configDialog(QWidget* parent, _FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig)
{
    KDialog* dialog;
    dialog = new KDialog(parent);
    FcitxConfigPage* configPage = new FcitxConfigPage(
        dialog,
        cfdesc,
        prefix,
        name,
        subconfig
    );
    dialog->setWindowIcon(KIcon("fcitx"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
    dialog->setMainWidget(configPage);
    connect(dialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), configPage, SLOT(buttonClicked(KDialog::ButtonCode)));

    return dialog;
}

KDialog* FcitxConfigPage::configDialog(QWidget* parent, FcitxAddon* addonEntry)
{
    if (!addonEntry)
        return NULL;

    KDialog* dialog;
    FcitxConfigFileDesc* cfdesc = ConfigDescManager::instance()->GetConfigDesc(QString::fromUtf8(addonEntry->name).append(".desc"));

    if (cfdesc ||  strlen(addonEntry->subconfig) != 0) {
        dialog = configDialog(
            parent,
            cfdesc,
            QString::fromUtf8("conf"),
            QString::fromUtf8(addonEntry->name).append(".config") ,
            QString::fromUtf8(addonEntry->subconfig)
        );
        return dialog;
    }

    return NULL;
}

void FcitxConfigPage::setupSubConfigUi()
{
    QStringList keys = m_parser->getSubConfigKeys();
    if (keys.length() != 0) {
        QWidget* main = new QWidget(this);
        QVBoxLayout* mainLayout = new QVBoxLayout;
        main->setLayout(mainLayout);

        QScrollArea *scrollarea = new QScrollArea;
        scrollarea->setFrameStyle(QFrame::NoFrame);
        scrollarea->setWidgetResizable(true);

        QWidget* form = new QWidget;
        QFormLayout* formLayout = new QFormLayout;
        scrollarea->setWidget(form);
        form->setLayout(formLayout);

        Q_FOREACH(const QString & key, keys) {
            FcitxSubConfig* subconfig = m_parser->getSubConfig(key);
            formLayout->addRow(
                QString::fromUtf8(
                    dgettext(m_parser->domain().toUtf8().data(),
                             subconfig->name().toUtf8().data()
                            )
                ),
                new FcitxSubConfigWidget(subconfig, this));
        }

        scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        scrollarea->setMinimumWidth(form->sizeHint().width() + 20);
        mainLayout->addWidget(scrollarea);
        m_ui->tabWidget->addTab(main, i18n("Other"));
    }
}

void SyncFilterFunc(FcitxGenericConfig* gconfig, FcitxConfigGroup *group, FcitxConfigOption *option, void *value, FcitxConfigSync sync, void *arg)
{
    Q_UNUSED(gconfig);
    Q_UNUSED(group);
    Q_UNUSED(value);
    FcitxConfigOptionDesc *codesc = option->optionDesc;

    if (!codesc)
        return;

    if (sync == Raw2Value) {
        switch (codesc->type) {

        case T_I18NString:
            break;

        case T_Integer: {
            int value = atoi(option->rawValue);
            QSpinBox* spinbox = static_cast<QSpinBox*>(arg);
            spinbox->setValue(value);
        }

        break;

        case T_Color: {
            int r = 0, g = 0, b = 0;
            sscanf(option->rawValue, "%d %d %d", &r, &g, &b);
            r = RoundColor(r);
            g = RoundColor(g);
            b = RoundColor(b);
            QColor color(r, g, b);
            KColorButton* colorButton = static_cast<KColorButton*>(arg);
            colorButton->setColor(color);
        }

        break;

        case T_Boolean: {
            bool bl;

            if (strcmp(option->rawValue, "True") == 0)
                bl = true;
            else
                bl = false;

            QCheckBox* checkBox = static_cast<QCheckBox*>(arg);

            checkBox->setChecked(bl);
        }

        break;

        case T_Font: {
            KFontComboBox *fontComboBox = static_cast<KFontComboBox*>(arg);
            QFont font(QString::fromUtf8(option->rawValue));
            fontComboBox->setCurrentFont(font);
        }

        break;

        case T_Enum: {
            FcitxConfigEnum* cenum = &codesc->configEnum;
            int index = 0, i;

            for (i = 0; i < cenum->enumCount; i++) {
                if (strcmp(cenum->enumDesc[i], option->rawValue) == 0) {
                    index = i;
                }
            }

            KComboBox* combobox = static_cast<KComboBox*>(arg);

            combobox->setCurrentIndex(index);
        }

        break;

        case T_Hotkey: {
            FcitxHotkey hotkey[2];

            FcitxHotkeySetKey(option->rawValue, hotkey);

            QHBoxLayout* hbox = static_cast<QHBoxLayout*>(arg);
            KKeySequenceWidget* keyseq[2];
            keyseq[0] = static_cast<KKeySequenceWidget*>(hbox->itemAt(0)->widget());
            keyseq[1] = static_cast<KKeySequenceWidget*>(hbox->itemAt(1)->widget());

            int j;
            for (j = 0; j < 2; j ++) {
                keyseq[j]->setKeySequence(HotkeyToKeySequence(&hotkey[j]));
                if (hotkey[j].desc)
                    free(hotkey[j].desc);
            }
        }

        break;

        case T_File: {
            KUrlRequester* urlrequester = static_cast<KUrlRequester*>(arg);
            urlrequester->setUrl(QString::fromUtf8(option->rawValue));
        }
        break;

        case T_Char:

        case T_String: {
            KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
            lineEdit->setText(QString::fromUtf8(option->rawValue));
        }

        break;
        }
    } else {
        if (codesc->type != T_I18NString && option->rawValue) {
            free(option->rawValue);
            option->rawValue = NULL;
        }

        switch (codesc->type) {

        case T_I18NString:
            break;

        case T_Integer: {
            int value;
            QSpinBox* spinbox = static_cast<QSpinBox*>(arg);
            value = spinbox->value();
            asprintf(&option->rawValue, "%d", value);
        }

        break;

        case T_Color: {
            int r = 0, g = 0, b = 0;
            QColor color;
            KColorButton* colorButton = static_cast<KColorButton*>(arg);
            color = colorButton->color();
            r = color.red();
            g = color.green();
            b = color.blue();
            r = RoundColor(r);
            g = RoundColor(g);
            b = RoundColor(b);
            asprintf(&option->rawValue, "%d %d %d", r, g, b);
        }

        break;

        case T_Boolean: {
            QCheckBox* checkBox = static_cast<QCheckBox*>(arg);
            bool bl;
            bl = checkBox->isChecked();

            if (bl)
                option->rawValue = strdup("True");
            else
                option->rawValue = strdup("False");
        }

        break;

        case T_Font: {
            KFontComboBox *fontComboBox = static_cast<KFontComboBox*>(arg);
            const QFont& font = fontComboBox->currentFont();
            option->rawValue = strdup(font.family().toUtf8().data());
        }

        break;

        case T_Enum: {
            KComboBox* combobox = static_cast<KComboBox*>(arg);
            FcitxConfigEnum* cenum = &codesc->configEnum;
            int index = 0;
            index = combobox->currentIndex();
            option->rawValue = strdup(cenum->enumDesc[index]);
        }

        break;

        case T_Hotkey: {
            QHBoxLayout* hbox = static_cast<QHBoxLayout*>(arg);
            KKeySequenceWidget* keyseq[2];
            keyseq[0] = static_cast<KKeySequenceWidget*>(hbox->itemAt(0)->widget());
            keyseq[1] = static_cast<KKeySequenceWidget*>(hbox->itemAt(1)->widget());
            char *strkey[2] = { NULL, NULL };
            int j = 0, k = 0;

            FcitxHotkey hotkey[2];

            for (j = 0; j < 2 ; j ++) {
                if (KeySequenceToHotkey(keyseq[j]->keySequence(), &hotkey[j])) {
                    strkey[k] = FcitxHotkeyGetKeyString(hotkey[j].sym, hotkey[j].state);
                    if (strkey[k])
                        k ++;
                }
            }
            if (strkey[1])
                asprintf(&option->rawValue, "%s %s", strkey[0], strkey[1]);
            else if (strkey[0]) {
                option->rawValue = strdup(strkey[0]);
            } else
                option->rawValue = strdup("");

            for (j = 0 ; j < k ; j ++)
                free(strkey[j]);
        }

        break;

        case T_File: {
            KUrlRequester* urlrequester = static_cast<KUrlRequester*>(arg);
            option->rawValue = strdup(urlrequester->url().toLocalFile().toUtf8().data());
        }
        break;

        case T_Char:

        case T_String: {
            KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
            option->rawValue = strdup(lineEdit->text().toUtf8().data());
        }

        break;
        }
    }
}


bool
KeySequenceToHotkey(const QKeySequence& keyseq, FcitxHotkey* hotkey)
{
    if (keyseq.count() != 1)
        return false;
    int key = keyseq[0] & (~Qt::KeyboardModifierMask);
    int state = keyseq[0] & Qt::KeyboardModifierMask;
    int sym = 0;
    keyQtToSym(key, Qt::KeyboardModifiers(state), sym, hotkey->state);
    hotkey->sym = (FcitxKeySym) sym;

    return true;
}

QKeySequence
HotkeyToKeySequence(FcitxHotkey* hotkey)
{
    int state = hotkey->state;
    FcitxKeySym keyval = hotkey->sym;

    Qt::KeyboardModifiers qstate = Qt::NoModifier;

    int key;
    symToKeyQt((int) keyval, state, key, qstate);

    return QKeySequence(key | qstate);
}

}

#include "moc_FcitxConfigPage.cpp"
