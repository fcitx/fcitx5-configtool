#include "FcitxConfigPage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QDebug>
#include <KColorButton>
#include <KFontComboBox>
#include <KComboBox>
#include <KKeySequenceWidget>
#include <KLineEdit>
#include <libintl.h>
#include <fcitx-config/fcitx-config.h>



#include "config.h"
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>

static 
void SyncFilterFunc(GenericConfig* gconfig, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg);

FcitxConfigPage::FcitxConfigPage(QWidget* parent, _ConfigFileDesc* cfdesc, const QString& prefix, const QString& name):
    QWidget(parent), m_cfdesc(cfdesc), m_prefix(prefix), m_name(name), m_ui(new Ui::FcitxConfigPage)
{
    m_ui->setupUi(this);
    
    bindtextdomain(m_cfdesc->domain, LOCALEDIR);
    bind_textdomain_codeset(m_cfdesc->domain, "UTF-8");
        
    FILE *fp;
    fp = GetXDGFileUserWithPrefix(prefix.toUtf8().data(), m_name.toUtf8().data(), "rt", NULL);
    ConfigFile *cfile = ParseConfigFileFp(fp, cfdesc);
    
    if (fp)
        fclose(fp);
    
    gconfig.configFile = cfile;
    
    ConfigGroupDesc *cgdesc = NULL;
    ConfigOptionDesc *codesc = NULL;
    for(cgdesc = cfdesc->groupsDesc;
        cgdesc != NULL;
        cgdesc = (ConfigGroupDesc*)cgdesc->hh.next)
    {
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
        for ( ; codesc != NULL;
            codesc = (ConfigOptionDesc*)codesc->hh.next, i++)
        {
            QString s;
            if (codesc->desc && strlen(codesc->desc) != 0)
                s = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->desc));
            else
                s = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->optionName));
            
            QWidget *inputWidget = NULL;
            void *argument = NULL;

            switch(codesc->type)
            {
                case T_Integer:
                    {
                        QSpinBox* spinbox = new QSpinBox(this);
                        spinbox->setMaximum(10000);
                        spinbox->setMinimum(-1);
                        inputWidget = spinbox;
                        argument = inputWidget;
                        connect(spinbox, SIGNAL(valueChanged(int)), this, SIGNAL(changed()));
                        break;
                    }
                case T_Color:
                    {
                        KColorButton* colorButton = new KColorButton(this);
                        inputWidget = colorButton;
                        argument = inputWidget;
                        connect(colorButton, SIGNAL(changed(QColor)), this, SIGNAL(changed()));
                    }
                    break;
                case T_Boolean:
                    {
                        QCheckBox* checkBox = new QCheckBox(this);
                        inputWidget = checkBox;
                        argument = inputWidget;
                        
                        connect(checkBox, SIGNAL(clicked(bool)), this, SIGNAL(changed()));
                        break;
                    }
                case T_Font:
                    {
                        KFontComboBox* fontComboBox = new KFontComboBox(this);
                        inputWidget = fontComboBox;
                        argument = inputWidget;
                        connect(fontComboBox, SIGNAL(currentFontChanged(QFont)), this, SIGNAL(changed()));
                    }
                    break;
                case T_Enum:
                    {
                        int i;
                        ConfigEnum *e = &codesc->configEnum;
                        KComboBox* combobox = new KComboBox(this);
                        inputWidget = combobox;
                        for (i = 0; i < e->enumCount; i ++)
                        {
                            combobox->addItem(QString::fromUtf8(dgettext(m_cfdesc->domain, e->enumDesc[i])));
                        }
                        argument = inputWidget;
                        connect(combobox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
                    }
                    break;
                case T_Hotkey:
                    {
                        KKeySequenceWidget* keyseq = new KKeySequenceWidget(this);
                        keyseq->setMultiKeyShortcutsAllowed(false);
                        keyseq->setModifierlessAllowed(true);
                        inputWidget = keyseq;
                        argument = inputWidget;
                        connect(keyseq, SIGNAL(keySequenceChanged(QKeySequence)), this, SIGNAL(changed()));
                    }
                    break;
                case T_File:
                case T_Char:
                case T_String:
                    {
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
            if (inputWidget)
            {
                QLabel* label = new QLabel(s, this);
                formLayout->addRow(label, inputWidget);
                ConfigBindValue(cfile, cgdesc->groupName, codesc->optionName, NULL, SyncFilterFunc, argument);
            }
        }
        
        scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollarea->setMinimumWidth(form->sizeHint().width() + 20);
        mainLayout->addWidget(scrollarea);
        
        m_ui->tabWidget->addTab(main, QString::fromUtf8(dgettext(m_cfdesc->domain, cgdesc->groupName)));
    }
    
    ConfigBindSync(&gconfig);
}

FcitxConfigPage::~FcitxConfigPage()
{

}

void FcitxConfigPage::buttonClicked(KDialog::ButtonCode code)
{
    if (code == KDialog::Default)
    {
        ResetConfigToDefaultValue(&this->gconfig);
        ConfigBindSync(&this->gconfig);
    }
    else if (code == KDialog::Ok)
    {
        FILE* fp = GetXDGFileUserWithPrefix(m_prefix.toUtf8().data(), m_name.toUtf8().data(), "wt", NULL);
        if (fp)
        {
            SaveConfigFileFp(fp, &gconfig, m_cfdesc);
            fclose(fp);
        }
    }
}


void SyncFilterFunc(GenericConfig* gconfig, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg)
{
    Q_UNUSED(gconfig);
    Q_UNUSED(group);
    Q_UNUSED(value);
    ConfigOptionDesc *codesc = option->optionDesc;
    if (!codesc)
        return;
    if (sync == Raw2Value)
    {
        switch (codesc->type)
        {
            case T_I18NString:
                break;            
            case T_Integer:
                {
                    int value = atoi(option->rawValue);
                    QSpinBox* spinbox = static_cast<QSpinBox*>(arg);
                    spinbox->setValue(value);
                }
                break;
            case T_Color:
                {
                    int r = 0,g = 0,b = 0;
                    sscanf(option->rawValue, "%d %d %d",&r,&g,&b);
                    r = RoundColor(r);
                    g = RoundColor(g);
                    b = RoundColor(b);
                    QColor color(r, g, b);
                    KColorButton* colorButton = static_cast<KColorButton*>(arg);
                    colorButton->setColor(color);
                }
                break;
            case T_Boolean:
                {
                    bool bl;
                    if (strcmp(option->rawValue, "True") == 0)
                        bl = true;
                    else
                        bl = false;

                    QCheckBox* checkBox = static_cast<QCheckBox*>(arg);
                    checkBox->setChecked(bl);
                }
                break;
            case T_Font:
                {
                    KFontComboBox *fontComboBox = static_cast<KFontComboBox*>(arg);
                    QFont font(QString::fromUtf8(option->rawValue));
                    fontComboBox->setCurrentFont(font);
                }
                break;
            case T_Enum:
                {
                    ConfigEnum* cenum = &codesc->configEnum;
                    int index = 0, i;
                    for (i = 0; i< cenum->enumCount; i++)
                    {
                        if ( strcmp(cenum->enumDesc[i], option->rawValue) == 0)
                        {
                            index = i;
                        }
                    }
                    KComboBox* combobox = static_cast<KComboBox*>(arg);
                    combobox->setCurrentIndex(index);
                }
                break;
            case T_Hotkey:
                {
                    HOTKEYS hotkey[2];
                    SetHotKey(option->rawValue, hotkey);
                }
                break;
            case T_File:
            case T_Char:
            case T_String:
                {
                    KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
                    lineEdit->setText(QString::fromUtf8(option->rawValue));
                }
                break;
        }
    }
    else
    {
        if (codesc->type != T_I18NString && option->rawValue)
        {
            free(option->rawValue);
            option->rawValue = NULL;
        }
        switch (codesc->type)
        {
            case T_I18NString:
                break;
            case T_Integer:
                {
                    int value;
                    QSpinBox* spinbox = static_cast<QSpinBox*>(arg);
                    value = spinbox->value();
                    asprintf(&option->rawValue, "%d", value);
                }
                break;
            case T_Color:
                {
                    int r = 0,g = 0,b = 0;
                    QColor color;
                    KColorButton* colorButton = static_cast<KColorButton*>(arg);
                    color = colorButton->color();
                    r = color.red();
                    g = color.green();
                    b = color.blue();
                    r = RoundColor(r);
                    g = RoundColor(g);
                    b = RoundColor(b);
                    asprintf(&option->rawValue, "%d %d %d",r,g,b);
                }
                break;
            case T_Boolean:
                {
                    QCheckBox* checkBox = static_cast<QCheckBox*>(arg);
                    bool bl;
                    bl = checkBox->isChecked();
                    if (bl)
                        option->rawValue = strdup("True");
                    else
                        option->rawValue = strdup("False");
                }
                break;
            case T_Font:
                {
                    KFontComboBox *fontComboBox = static_cast<KFontComboBox*>(arg);
                    const QFont& font = fontComboBox->currentFont();
                    option->rawValue = strdup(font.family().toUtf8().data());
                }
                break;
            case T_Enum:
                {
                    KComboBox* combobox = static_cast<KComboBox*>(arg);
                    ConfigEnum* cenum = &codesc->configEnum;
                    int index = 0;
                    index = combobox->currentIndex();
                    option->rawValue = strdup(cenum->enumDesc[index]);
                }
                break;
            case T_Hotkey:
                {
                    option->rawValue = strdup("");
                }
                break;
            case T_File:
            case T_Char:
            case T_String:
                {
                    KLineEdit* lineEdit = static_cast<KLineEdit*>(arg);
                    option->rawValue = strdup(lineEdit->text().toUtf8().data());
                }
                break;
        }
    }
}

#include "moc_FcitxConfigPage.cpp"