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

// KDE
#include <KColorButton>
#include <KComboBox>
#include <KLineEdit>
#include <KRun>
#include <KPushButton>
#include <KMessageBox>
#include <KUrlRequester>
#include <KLocalizedString>

// system
#include <libintl.h>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>
#include <fcitx-qt/fcitxqtkeysequencewidget.h>

// self
#include "config.h"
#include "configwidget.h"
#include "subconfigparser.h"
#include "subconfigwidget.h"
#include "global.h"
#include "dummyconfig.h"
#include "verticalscrollarea.h"
#include "fontbutton.h"

#define RoundColor(c) ((c)>=0?((c)<=255?c:255):0)

namespace Fcitx
{

static bool KeySequenceToHotkey(const QKeySequence& keyseq, FcitxQtModifierSide side, FcitxHotkey* hotkey);
static QKeySequence HotkeyToKeySequence(FcitxHotkey* hotkey);

static
void SyncFilterFunc(FcitxGenericConfig* gconfig, FcitxConfigGroup *group, FcitxConfigOption *option, void *value, FcitxConfigSync sync, void *arg);

ConfigWidget::ConfigWidget(FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig, const QString& addonName, QWidget* parent) : QWidget(parent)
    , m_cfdesc(cfdesc)
    , m_prefix(prefix)
    , m_name(name)
    , m_addonName(addonName)
    , m_switchLayout(new QVBoxLayout)
    , m_simpleWidget(0)
    , m_fullWidget(0)
    , m_advanceCheckBox(0)
    , m_config(0)
    , m_parser(new SubConfigParser(subconfig, this))
    , m_simpleUiType(CW_NoShow)
    , m_fullUiType(CW_NoShow)
{
    if (cfdesc)
        m_config = new DummyConfig(cfdesc);
    setupConfigUi();
}

ConfigWidget::ConfigWidget(FcitxAddon* addonEntry, QWidget* parent): QWidget(parent)
    , m_cfdesc(Global::instance()->GetConfigDesc(QString::fromUtf8(addonEntry->name).append(".desc")))
    , m_prefix("conf")
    , m_name(QString::fromUtf8(addonEntry->name).append(".config"))
    , m_addonName(addonEntry->name)
    , m_switchLayout(new QVBoxLayout)
    , m_simpleWidget(0)
    , m_fullWidget(0)
    , m_advanceCheckBox(0)
    , m_config(0)
    , m_parser(new SubConfigParser(QString::fromUtf8(addonEntry->subconfig), this))
    , m_simpleUiType(CW_NoShow)
    , m_fullUiType(CW_NoShow)
{
    if (m_cfdesc)
        m_config = new DummyConfig(m_cfdesc);
    setupConfigUi();
}


ConfigWidget::~ConfigWidget()
{
    if (m_config)
        delete m_config;
}

void ConfigWidget::buttonClicked(KDialog::ButtonCode code)
{
    if (!m_cfdesc)
        return;

    if (code == KDialog::Default) {
        FcitxConfigResetConfigToDefaultValue(m_config->genericConfig());
        FcitxConfigBindSync(m_config->genericConfig());
    } else if (code == KDialog::Ok) {
        FILE* fp = FcitxXDGGetFileUserWithPrefix(m_prefix.toLocal8Bit().constData(), m_name.toLocal8Bit().constData(), "w", NULL);

        if (fp) {
            FcitxConfigSaveConfigFileFp(fp, m_config->genericConfig(), m_cfdesc);
            fclose(fp);
        }

        if (Global::instance()->inputMethodProxy()) {
            if (m_addonName.isEmpty()) {
                Global::instance()->inputMethodProxy()->ReloadConfig();
            } else {
                Global::instance()->inputMethodProxy()->ReloadAddonConfig(m_addonName);
            }
        }
    }
}

void ConfigWidget::load()
{
    if (!m_cfdesc)
        return;
    FILE *fp;
    fp = FcitxXDGGetFileWithPrefix(m_prefix.toLocal8Bit().constData(), m_name.toLocal8Bit().constData(), "r", NULL);
    if (!fp)
        return;

    m_config->load(fp);
    m_config->sync();
    fclose(fp);
}

void ConfigWidget::createConfigOptionWidget(FcitxConfigGroupDesc* cgdesc, FcitxConfigOptionDesc* codesc, QString& label, QString& tooltip, QWidget*& inputWidget, void*& newarg)
{
    FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;

    void* oldarg = NULL;
    void* argument = NULL;
    QString name(QString("%1/%2").arg(cgdesc->groupName).arg(codesc->optionName));
    if (m_argMap.contains(name)) {
        oldarg = m_argMap[name];
    }

    if (codesc->desc && codesc->desc[0])
        label = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->desc));
    else
        label = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->optionName));

    if (codesc2->longDesc && codesc2->longDesc[0]) {
        tooltip = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc2->longDesc));
    }

    switch (codesc->type) {

    case T_Integer: {
        QSpinBox* spinbox = new QSpinBox(this);
        spinbox->setMaximum(codesc2->constrain.integerConstrain.max);
        spinbox->setMinimum(codesc2->constrain.integerConstrain.min);
        inputWidget = spinbox;
        if (!oldarg) {
            connect(spinbox, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            QSpinBox* oldspinbox = (QSpinBox*) oldarg;
            connect(oldspinbox, SIGNAL(valueChanged(int)), spinbox, SLOT(setValue(int)));
            connect(spinbox, SIGNAL(valueChanged(int)), oldspinbox, SLOT(setValue(int)));
        }
        break;
    }

    case T_Color: {
        ColorButton* colorButton = new ColorButton(this);
        inputWidget = colorButton;
        if (!oldarg) {
            connect(colorButton, SIGNAL(changed(QColor)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            ColorButton* oldColorButton = (ColorButton*) oldarg;
            connect(colorButton, SIGNAL(changed(QColor)), oldColorButton, SLOT(setColor(QColor)));
            connect(oldColorButton, SIGNAL(changed(QColor)), colorButton, SLOT(setColor(QColor)));
        }
    }

    break;

    case T_Boolean: {
        QCheckBox* checkBox = new QCheckBox(this);
        inputWidget = checkBox;

        if (!oldarg) {
            connect(checkBox, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            QCheckBox* oldCheckBox = (QCheckBox*) oldarg;
            connect(checkBox, SIGNAL(toggled(bool)), oldCheckBox, SLOT(setChecked(bool)));
            connect(oldCheckBox, SIGNAL(toggled(bool)), checkBox, SLOT(setChecked(bool)));
        }
        break;
    }

    case T_Font: {
        FontButton* fontButton = new FontButton(this);
        inputWidget = fontButton;
        if (!oldarg) {
            connect(fontButton, SIGNAL(fontChanged(QFont)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            FontButton* oldFontButton = (FontButton*) oldarg;
            connect(fontButton, SIGNAL(fontChanged(QFont)), oldFontButton, SLOT(setFont(QFont)));
            connect(oldFontButton, SIGNAL(fontChanged(QFont)), fontButton, SLOT(setFont(QFont)));
        }
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

        if (!oldarg) {
            connect(combobox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            KComboBox* oldComboBox = (KComboBox*) oldarg;
            connect(combobox, SIGNAL(currentIndexChanged(int)), oldComboBox, SLOT(setCurrentIndex(int)));
            connect(oldComboBox, SIGNAL(currentIndexChanged(int)), combobox, SLOT(setCurrentIndex(int)));
        }

    }

    break;

    case T_Hotkey: {
        FcitxQtKeySequenceWidget* keyseq1 = new FcitxQtKeySequenceWidget();
        FcitxQtKeySequenceWidget* keyseq2 = new FcitxQtKeySequenceWidget();
        QHBoxLayout* hbox = new QHBoxLayout();
        hbox->setMargin(0);
        QWidget* widget = new QWidget(this);
        keyseq1->setMultiKeyShortcutsAllowed(false);
        keyseq1->setModifierOnlyAllowed(codesc2->constrain.hotkeyConstrain.allowModifierOnly);
        keyseq1->setModifierlessAllowed(!codesc2->constrain.hotkeyConstrain.disallowNoModifer);
        keyseq1->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        keyseq2->setMultiKeyShortcutsAllowed(false);
        keyseq2->setModifierOnlyAllowed(codesc2->constrain.hotkeyConstrain.allowModifierOnly);
        keyseq1->setModifierlessAllowed(!codesc2->constrain.hotkeyConstrain.disallowNoModifer);
        keyseq2->setModifierlessAllowed(true);
        keyseq2->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        hbox->addWidget(keyseq1);
        hbox->addWidget(keyseq2);
        widget->setLayout(hbox);

        inputWidget = widget;

        if (!oldarg) {
            connect(keyseq1, SIGNAL(keySequenceChanged(QKeySequence,FcitxQtModifierSide)), this, SIGNAL(changed()));
            connect(keyseq2, SIGNAL(keySequenceChanged(QKeySequence,FcitxQtModifierSide)), this, SIGNAL(changed()));
            argument = hbox;
        }
        else {
            QHBoxLayout* hbox = static_cast<QHBoxLayout*>(oldarg);
            FcitxQtKeySequenceWidget* oldkeyseq1 = static_cast<FcitxQtKeySequenceWidget*>(hbox->itemAt(0)->widget());
            FcitxQtKeySequenceWidget* oldkeyseq2 = static_cast<FcitxQtKeySequenceWidget*>(hbox->itemAt(1)->widget());
            connect(oldkeyseq1, SIGNAL(keySequenceChanged(QKeySequence, FcitxQtModifierSide)),
                    keyseq1,    SLOT(setKeySequence(QKeySequence, FcitxQtModifierSide)));
            connect(oldkeyseq2, SIGNAL(keySequenceChanged(QKeySequence, FcitxQtModifierSide)),
                    keyseq2,    SLOT(setKeySequence(QKeySequence, FcitxQtModifierSide)));
            connect(keyseq1,    SIGNAL(keySequenceChanged(QKeySequence, FcitxQtModifierSide)),
                    oldkeyseq1, SLOT(setKeySequence(QKeySequence, FcitxQtModifierSide)));
            connect(keyseq2,    SIGNAL(keySequenceChanged(QKeySequence, FcitxQtModifierSide)),
                    oldkeyseq2, SLOT(setKeySequence(QKeySequence, FcitxQtModifierSide)));
        }
    }

    break;

    case T_File: {
        KUrlRequester* requester = new KUrlRequester(this);
        inputWidget = requester;

        if (!oldarg) {
            connect(requester, SIGNAL(urlSelected(KUrl)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            KUrlRequester* oldrequester = static_cast<KUrlRequester*>(oldarg);
            connect(requester, SIGNAL(urlSelected(KUrl)), oldrequester, SLOT(setUrl(KUrl)));
            connect(oldrequester, SIGNAL(urlSelected(KUrl)), requester, SLOT(setUrl(KUrl)));
        }
    }
    break;

    case T_Char:

    case T_String: {
        KLineEdit* lineEdit = new KLineEdit(this);
        inputWidget = lineEdit;
        argument = inputWidget;

        if (codesc->type == T_Char)
            lineEdit->setMaxLength(1);
        else if (codesc->type == T_String
            && codesc2->constrain.stringConstrain.maxLength) {
            lineEdit->setMaxLength(codesc2->constrain.stringConstrain.maxLength);
        }

        if (!oldarg) {
            connect(lineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
            argument = inputWidget;
        }
        else {
            KLineEdit* oldLineEdit = static_cast<KLineEdit*>(oldarg);
            connect(lineEdit, SIGNAL(textChanged(QString)), oldLineEdit, SLOT(setText(QString)));
            connect(oldLineEdit, SIGNAL(textChanged(QString)), lineEdit, SLOT(setText(QString)));
        }

    }

    break;

    case T_I18NString:
        inputWidget = NULL;
        argument = NULL;
        break;
    }

    if (inputWidget && !tooltip.isEmpty())
        inputWidget->setToolTip(tooltip);

    if (argument) {
        m_argMap[name] = argument;
        newarg = argument;
    }
}

QWidget* ConfigWidget::createSimpleConfigUi(bool skipAdvance)
{
    int row = 0;
    VerticalScrollArea *scrollarea = new VerticalScrollArea;
    scrollarea->setFrameStyle(QFrame::NoFrame);
    scrollarea->setWidgetResizable(true);

    QWidget* form = new QWidget;
    QGridLayout* gridLayout = new QGridLayout;
    scrollarea->setWidget(form);
    form->setLayout(gridLayout);

    do {
        if (!m_cfdesc)
            break;

        if (!m_config->isValid())
            break;

        HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            if (cgdesc->optionsDesc == NULL)
                continue;
            else {
                int count = 0;
                HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                    FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
                    if (!skipAdvance || !codesc2->advance)
                        count++;
                }
                if (!count)
                    continue;
            }
            QLabel* grouplabel = new QLabel(QString("<b>%1</b>").arg(QString::fromUtf8(dgettext(m_cfdesc->domain, cgdesc->groupName))));
            gridLayout->addWidget(grouplabel, row++, 0, 1, 3);

            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
                if (skipAdvance && codesc2->advance)
                    continue;
                QString s, tooltip;
                QWidget* inputWidget = NULL;
                void* argument = NULL;
                createConfigOptionWidget(cgdesc, codesc, s, tooltip, inputWidget, argument);

                if (inputWidget) {
                    QLabel* label = new QLabel(s);
                    if (!tooltip.isEmpty())
                        label->setToolTip(tooltip);
                    gridLayout->addWidget(label, row, 1, Qt::AlignCenter | Qt::AlignRight);
                    gridLayout->addWidget(inputWidget, row, 2);
                    if (argument)
                        m_config->bind(cgdesc->groupName, codesc->optionName, SyncFilterFunc, argument);
                    row++;
                }
            }
        }
    } while(0);

    QStringList keys = m_parser->getSubConfigKeys();
    if (keys.length() != 0) {
        int labelrow = row;
        row ++;
        Q_FOREACH(const QString & key, keys) {
            SubConfig* subconfig = m_parser->getSubConfig(key);
            if (!subconfig) {
                continue;
            }
            if (!subconfig->isValid()) {
                delete subconfig;
                continue;
            }

            QLabel* label = new QLabel(QString::fromUtf8(
                    dgettext(m_parser->domain().toUtf8().constData(),
                                subconfig->name().toUtf8().constData()
                            )
                ));
            gridLayout->addWidget(label, row, 1, Qt::AlignCenter | Qt::AlignRight);
            gridLayout->addWidget(new SubConfigWidget(subconfig, this), row, 2);
            row++;
        }
        if (row != labelrow + 1) {
            QLabel* grouplabel = new QLabel(i18n("<b>Other</b>"));
            gridLayout->addWidget(grouplabel, labelrow, 0, 1, 3);
        }
    }

    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    if (row >= 2) {
        QSpacerItem* horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
        gridLayout->addItem(horizontalSpacer, 2, 0, 1, 1);
    }

    gridLayout->addItem(verticalSpacer, row, 1, 1, 1);

    return scrollarea;
}

QWidget* ConfigWidget::createFullConfigUi()
{
    QTabWidget* tabWidget = new QTabWidget(this);
    do {
        if (!m_cfdesc)
            break;

        if (!m_config->isValid())
            break;

        HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            if (cgdesc->optionsDesc == NULL)
                continue;

            QWidget* main = new QWidget(this);
            QVBoxLayout* mainLayout = new QVBoxLayout;
            main->setLayout(mainLayout);

            VerticalScrollArea *scrollarea = new VerticalScrollArea;
            scrollarea->setFrameStyle(QFrame::NoFrame);
            scrollarea->setWidgetResizable(true);

            QWidget* form = new QWidget;
            QFormLayout* formLayout = new QFormLayout;
            scrollarea->setWidget(form);
            form->setLayout(formLayout);

            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                QString s, tooltip;
                QWidget* inputWidget = NULL;
                void* argument = NULL;
                createConfigOptionWidget(cgdesc, codesc, s, tooltip, inputWidget, argument);

                if (inputWidget) {
                    QLabel* label = new QLabel(s, this);
                    if (!tooltip.isEmpty())
                        label->setToolTip(tooltip);
                    formLayout->addRow(label, inputWidget);
                    if (argument)
                        m_config->bind(cgdesc->groupName, codesc->optionName, SyncFilterFunc, argument);
                }
            }

            mainLayout->addWidget(scrollarea);

            tabWidget->addTab(main, QString::fromUtf8(dgettext(m_cfdesc->domain, cgdesc->groupName)));
        }
    } while(0);

    QStringList keys = m_parser->getSubConfigKeys();
    if (keys.length()) {
        QList<SubConfig*> subConfigs;
        Q_FOREACH(const QString & key, keys) {
            SubConfig* subconfig = m_parser->getSubConfig(key);
            if (!subconfig) {
                continue;
            }
            if (!subconfig->isValid()) {
                delete subconfig;
                continue;
            }

            subConfigs << subconfig;
        }

        if (subConfigs.length() > 0) {
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

            Q_FOREACH(SubConfig* subconfig, subConfigs) {
                formLayout->addRow(
                    QString::fromUtf8(
                        dgettext(m_parser->domain().toUtf8().constData(),
                                    subconfig->name().toUtf8().constData()
                                )
                    ),
                    new SubConfigWidget(subconfig, this));
            }

            scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            scrollarea->setMinimumWidth(form->sizeHint().width() + 20);
            mainLayout->addWidget(scrollarea);
            tabWidget->addTab(main, i18n("Other"));
        }
    }

    return tabWidget;
}

void ConfigWidget::setupConfigUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(m_switchLayout);
    setLayout(layout);

    checkCanUseSimple();

    if (m_cfdesc) {
        bindtextdomain(m_cfdesc->domain, LOCALEDIR);
        bind_textdomain_codeset(m_cfdesc->domain, "UTF-8");
        FILE *fp;
        fp = FcitxXDGGetFileWithPrefix(m_prefix.toLocal8Bit().constData(), m_name.toLocal8Bit().constData(), "r", NULL);
        m_config->load(fp);

        if (fp)
            fclose(fp);
    }
    if (m_simpleUiType != CW_NoShow) {
        if (m_simpleUiType == CW_Simple)
            m_simpleWidget = createSimpleConfigUi(true);
        else
            m_simpleWidget = createFullConfigUi();
        m_switchLayout->addWidget(m_simpleWidget);
    }

    if (m_fullUiType != CW_NoShow) {
        if (m_fullUiType == CW_Simple)
            m_fullWidget = createSimpleConfigUi(false);
        else
            m_fullWidget = createFullConfigUi();
        m_switchLayout->addWidget(m_fullWidget);
    }

    if (m_simpleWidget && m_fullWidget)
    {
        m_advanceCheckBox = new QCheckBox(this);
        layout->addWidget(m_advanceCheckBox);
        m_advanceCheckBox->setCheckState(Qt::Unchecked);
        m_advanceCheckBox->setText(i18n("Show &Advance option"));
        connect(m_advanceCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleSimpleFull()));
        toggleSimpleFull();
    }

    if (m_config)
        m_config->sync();
}

void ConfigWidget::toggleSimpleFull()
{
    if (m_advanceCheckBox->isChecked()) {
        m_simpleWidget->hide();
        m_fullWidget->show();
    }
    else {
        m_simpleWidget->show();
        m_fullWidget->hide();
    }

}

void ConfigWidget::checkCanUseSimple()
{
    int count = 0;
    int simpleCount = 0;
    if (m_cfdesc) {
        HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            if (cgdesc->optionsDesc == NULL)
                continue;
            else {
                HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                    FcitxConfigOptionDesc2* codesc2 = (FcitxConfigOptionDesc2*) codesc;
                    if (!codesc2->advance)
                        simpleCount++;
                    count ++;
                }
            }
        }
    }

    /* if option is quite few */
    if (count + m_parser->getSubConfigKeys().length() <= 10) {
        m_fullUiType = CW_Simple;
    }
    else {
        m_fullUiType = CW_Full;
    }
    if (simpleCount + m_parser->getSubConfigKeys().length() <= 10) {
        m_simpleUiType = CW_Simple;
    }
    else
        m_simpleUiType = CW_Full;

    if (count == simpleCount)
        m_simpleUiType = CW_NoShow;
}


KDialog* ConfigWidget::configDialog(QWidget* parent, FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig, const QString& addonName)
{
    KDialog* dialog;
    dialog = new KDialog(parent);
    ConfigWidget* configPage = new ConfigWidget(
        cfdesc,
        prefix,
        name,
        subconfig,
        addonName,
        dialog
    );
    dialog->setWindowIcon(KIcon("fcitx"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
    dialog->setMainWidget(configPage);
    connect(dialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), configPage, SLOT(buttonClicked(KDialog::ButtonCode)));

    return dialog;
}

KDialog* ConfigWidget::configDialog(QWidget* parent, FcitxAddon* addonEntry)
{
    if (!addonEntry)
        return NULL;

    KDialog* dialog;
    FcitxConfigFileDesc* cfdesc = Global::instance()->GetConfigDesc(QString::fromUtf8(addonEntry->name).append(".desc"));

    if (cfdesc ||  strlen(addonEntry->subconfig) != 0) {
        dialog = configDialog(
            parent,
            cfdesc,
            QString::fromUtf8("conf"),
            QString::fromUtf8(addonEntry->name).append(".config") ,
            QString::fromUtf8(addonEntry->subconfig),
            QString::fromUtf8(addonEntry->name)
        );
        return dialog;
    }

    return NULL;
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
            int i = *(int*) value;
            QSpinBox* spinbox = static_cast<QSpinBox*>(arg);
            spinbox->setValue(i);
        }

        break;

        case T_Color: {
            FcitxConfigColor* rawcolor = (FcitxConfigColor*) value;
            QColor color(QColor::fromRgbF(rawcolor->r, rawcolor->g, rawcolor->b));
            ColorButton* colorButton = static_cast<ColorButton*>(arg);
            colorButton->setColor(color);
        }

        break;

        case T_Boolean: {
            boolean *bl = (boolean*) value;

            QCheckBox* checkBox = static_cast<QCheckBox*>(arg);

            checkBox->setChecked(*bl);
        }

        break;

        case T_Font: {
            char** fontname = (char**) value;
            FontButton *fontButton = static_cast<FontButton*>(arg);
            QFont font = FontButton::parseFont(QString::fromUtf8(*fontname));
            fontButton->setFont(font);
        }

        break;

        case T_Enum: {
            int index = *(int*) value;

            KComboBox* combobox = static_cast<KComboBox*>(arg);

            combobox->setCurrentIndex(index);
        }

        break;

        case T_Hotkey: {
            FcitxHotkey* hotkey = (FcitxHotkey*) value;

            QHBoxLayout* hbox = static_cast<QHBoxLayout*>(arg);
            FcitxQtKeySequenceWidget* keyseq[2];
            keyseq[0] = static_cast<FcitxQtKeySequenceWidget*>(hbox->itemAt(0)->widget());
            keyseq[1] = static_cast<FcitxQtKeySequenceWidget*>(hbox->itemAt(1)->widget());

            int j;
            for (j = 0; j < 2; j ++) {
                FcitxQtModifierSide side = MS_Unknown;
                if (hotkey[j].sym == FcitxKey_Control_L
                 || hotkey[j].sym == FcitxKey_Alt_L
                 || hotkey[j].sym == FcitxKey_Shift_L
                 || hotkey[j].sym == FcitxKey_Super_L) {
                    side = MS_Left;
                }
                if (hotkey[j].sym == FcitxKey_Control_R
                 || hotkey[j].sym == FcitxKey_Alt_R
                 || hotkey[j].sym == FcitxKey_Shift_R
                 || hotkey[j].sym == FcitxKey_Super_R) {
                    side = MS_Right;
                }
                keyseq[j]->setKeySequence(HotkeyToKeySequence(&hotkey[j]), side);
            }
        }

        break;

        case T_File: {
            char** filename = (char**) value;
            KUrlRequester* urlrequester = static_cast<KUrlRequester*>(arg);
            urlrequester->setUrl(QString::fromUtf8(*filename));
        }
        break;

        case T_Char: {
            char* string = (char*) value;
            char temp[2] = { *string, '\0' };
            KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
            lineEdit->setText(QString::fromUtf8(temp));
        }
        break;

        case T_String: {
            char** string = (char**) value;
            KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
            lineEdit->setText(QString::fromUtf8(*string));
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
            int* i = (int*) value;
            QSpinBox* spinbox = static_cast<QSpinBox*>(arg);
            *i = spinbox->value();
        }

        break;

        case T_Color: {
            QColor color;
            ColorButton* colorButton = static_cast<ColorButton*>(arg);
            color = colorButton->color();
            FcitxConfigColor* rawcolor = (FcitxConfigColor*) value;
            rawcolor->r = color.redF();
            rawcolor->g = color.greenF();
            rawcolor->b = color.blueF();
        }
        break;

        case T_Boolean: {
            QCheckBox* checkBox = static_cast<QCheckBox*>(arg);
            boolean* bl = (boolean*) value;
            *bl = checkBox->isChecked();
        }

        break;

        case T_Font: {
            FontButton *fontButton = static_cast<FontButton*>(arg);
            const QString font = fontButton->fontName();
            char** fontname = (char**) value;
            fcitx_utils_string_swap(fontname, font.toUtf8().constData());
        }

        break;

        case T_Enum: {
            KComboBox* combobox = static_cast<KComboBox*>(arg);
            int* index = (int*) value;
            *index = combobox->currentIndex();
        }

        break;

        case T_Hotkey: {
            QHBoxLayout* hbox = static_cast<QHBoxLayout*>(arg);
            FcitxQtKeySequenceWidget* keyseq[2];
            keyseq[0] = static_cast<FcitxQtKeySequenceWidget*>(hbox->itemAt(0)->widget());
            keyseq[1] = static_cast<FcitxQtKeySequenceWidget*>(hbox->itemAt(1)->widget());
            int j = 0;

            FcitxHotkey* hotkey = (FcitxHotkey*) value;

            for (j = 0; j < 2 ; j ++) {
                if (KeySequenceToHotkey(keyseq[j]->keySequence(), keyseq[j]->modifierSide(), &hotkey[j])) {
                    char* keystring = FcitxHotkeyGetKeyString(hotkey[j].sym, hotkey[j].state);
                    fcitx_utils_string_swap(&hotkey[j].desc, keystring);
                    fcitx_utils_free(keystring);
                }
                else {
                    fcitx_utils_string_swap(&hotkey[j].desc, NULL);
                }
            }
        }

        break;

        case T_File: {
            KUrlRequester* urlrequester = static_cast<KUrlRequester*>(arg);
            char** filename = (char**) value;
            fcitx_utils_string_swap(filename, urlrequester->url().toLocalFile().toUtf8().constData());
        }
        break;

        case T_Char: {
            KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
            char* c = (char*) value;
            *c = lineEdit->text()[0].toAscii();
        }
        break;

        case T_String: {
            KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
            char** string = (char**) value;
            fcitx_utils_string_swap(string, lineEdit->text().toUtf8().constData());
        }
        break;
        }
    }
}


bool
KeySequenceToHotkey(const QKeySequence& keyseq, FcitxQtModifierSide side, FcitxHotkey* hotkey)
{
    if (keyseq.count() != 1)
        return false;

    int sym = 0;
    FcitxQtKeySequenceWidget::keyQtToFcitx(keyseq[0], side, sym, hotkey->state);
    hotkey->sym = (FcitxKeySym) sym;

    return true;
}

QKeySequence
HotkeyToKeySequence(FcitxHotkey* hotkey)
{
    uint state = hotkey->state;
    FcitxKeySym keyval = hotkey->sym;

    return QKeySequence(FcitxQtKeySequenceWidget::keyFcitxToQt((int) keyval, state));
}

}
