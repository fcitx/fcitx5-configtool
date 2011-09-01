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

#include <KColorButton>
#include <KFontComboBox>
#include <KComboBox>
#include <KKeySequenceWidget>
#include <KLineEdit>
#include <KRun>
#include <KPushButton>
#include <KMessageBox>

#include <libintl.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>

#include "FcitxConfigPage.h"

#include "config.h"
#include "FcitxSubConfigParser.h"
#include "FcitxSubConfigWidget.h"
#include "keyserver_x11.h"
#include "ConfigDescManager.h"

namespace Fcitx
{
    static bool KeySequenceToHotkey(const QKeySequence& keyseq, HOTKEYS* hotkey);
    static QKeySequence HotkeyToKeySequence(HOTKEYS* hotkey);

    static
    void SyncFilterFunc ( GenericConfig* gconfig, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg );

    FcitxConfigPage::FcitxConfigPage ( QWidget* parent, _ConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig ) :
            QWidget ( parent ), m_cfdesc ( cfdesc ), m_prefix ( prefix ), m_name ( name ), m_ui ( new Ui::FcitxConfigPage ), m_parser( NULL )
    {
        m_parser = new FcitxSubConfigParser(subconfig, this);
        gconfig.configFile = NULL;
        m_ui->setupUi ( this );
        setupConfigUi();
        setupSubConfigUi();
    }

    FcitxConfigPage::~FcitxConfigPage()
    {
        FreeConfigFile(gconfig.configFile);
        delete m_ui;
    }

    void FcitxConfigPage::buttonClicked ( KDialog::ButtonCode code )
    {
        if (!m_cfdesc)
            return;

        if ( code == KDialog::Default )
        {
            ResetConfigToDefaultValue ( &this->gconfig );
            ConfigBindSync ( &this->gconfig );
        }
        else if ( code == KDialog::Ok )
        {
            FILE* fp = GetXDGFileUserWithPrefix ( m_prefix.toUtf8().data(), m_name.toUtf8().data(), "wt", NULL );

            if ( fp )
            {
                SaveConfigFileFp ( fp, &gconfig, m_cfdesc );
                fclose ( fp );
            }

            const char* reload_config = "kcm_fcitx_reload_config";
            KMessageBox::information(this,
                                    i18n("Not all configuration can be reloaded immediately."),
                                    i18n("Attention"),
                                    reload_config
                                );

            QStringList commandAndParameters;
            commandAndParameters <<"-r";
            QProcess process;
            process.startDetached(FCITX4_EXEC_PREFIX "/bin/fcitx-remote", commandAndParameters);
        }
    }

    void FcitxConfigPage::setupConfigUi()
    {
        if (m_cfdesc)
        {
            bindtextdomain ( m_cfdesc->domain, LOCALEDIR );
            bind_textdomain_codeset ( m_cfdesc->domain, "UTF-8" );

            FILE *fp;
            fp = GetXDGFileWithPrefix ( m_prefix.toUtf8().data(), m_name.toUtf8().data(), "rt", NULL );
            ConfigFile *cfile = ParseConfigFileFp ( fp, m_cfdesc );

            if ( fp )
                fclose ( fp );

            if (!cfile)
                return;

            gconfig.configFile = cfile;

            ConfigGroupDesc *cgdesc = NULL;

            ConfigOptionDesc *codesc = NULL;

            for ( cgdesc = m_cfdesc->groupsDesc;
                    cgdesc != NULL;
                    cgdesc = ( ConfigGroupDesc* ) cgdesc->hh.next )
            {
                codesc = cgdesc->optionsDesc;

                if ( codesc == NULL )
                    continue;

                QWidget* main = new QWidget ( this );

                QVBoxLayout* mainLayout = new QVBoxLayout;

                main->setLayout ( mainLayout );

                QScrollArea *scrollarea = new QScrollArea;

                scrollarea->setFrameStyle ( QFrame::NoFrame );

                scrollarea->setWidgetResizable ( true );

                QWidget* form = new QWidget;

                QFormLayout* formLayout = new QFormLayout;

                scrollarea->setWidget ( form );

                form->setLayout ( formLayout );

                int i = 0;

                for ( ; codesc != NULL;
                        codesc = ( ConfigOptionDesc* ) codesc->hh.next, i++ )
                {
                    QString s;

                    if ( codesc->desc && strlen ( codesc->desc ) != 0 )
                        s = QString::fromUtf8 ( dgettext ( m_cfdesc->domain, codesc->desc ) );
                    else
                        s = QString::fromUtf8 ( dgettext ( m_cfdesc->domain, codesc->optionName ) );

                    QWidget *inputWidget = NULL;

                    void *argument = NULL;

                    switch ( codesc->type )
                    {

                    case T_Integer:
                        {
                            QSpinBox* spinbox = new QSpinBox ( this );
                            spinbox->setMaximum ( 10000 );
                            spinbox->setMinimum ( -1 );
                            inputWidget = spinbox;
                            argument = inputWidget;
                            connect ( spinbox, SIGNAL ( valueChanged ( int ) ), this, SIGNAL ( changed() ) );
                            break;
                        }

                    case T_Color:
                        {
                            KColorButton* colorButton = new KColorButton ( this );
                            inputWidget = colorButton;
                            argument = inputWidget;
                            connect ( colorButton, SIGNAL ( changed ( QColor ) ), this, SIGNAL ( changed() ) );
                        }

                        break;

                    case T_Boolean:
                        {
                            QCheckBox* checkBox = new QCheckBox ( this );
                            inputWidget = checkBox;
                            argument = inputWidget;

                            connect ( checkBox, SIGNAL ( clicked ( bool ) ), this, SIGNAL ( changed() ) );
                            break;
                        }

                    case T_Font:
                        {
                            KFontComboBox* fontComboBox = new KFontComboBox ( this );
                            inputWidget = fontComboBox;
                            argument = inputWidget;
                            connect ( fontComboBox, SIGNAL ( currentFontChanged ( QFont ) ), this, SIGNAL ( changed() ) );
                        }

                        break;

                    case T_Enum:
                        {
                            int i;
                            ConfigEnum *e = &codesc->configEnum;
                            KComboBox* combobox = new KComboBox ( this );
                            inputWidget = combobox;

                            for ( i = 0; i < e->enumCount; i ++ )
                            {
                                combobox->addItem ( QString::fromUtf8 ( dgettext ( m_cfdesc->domain, e->enumDesc[i] ) ) );
                            }

                            argument = inputWidget;

                            connect ( combobox, SIGNAL ( currentIndexChanged ( int ) ), this, SIGNAL ( changed() ) );
                        }

                        break;

                    case T_Hotkey:
                        {
                            KKeySequenceWidget* keyseq1 = new KKeySequenceWidget (  );
                            KKeySequenceWidget* keyseq2 = new KKeySequenceWidget (  );
                            QHBoxLayout* hbox = new QHBoxLayout();
                            QWidget* widget = new QWidget(this);
                            keyseq1->setMultiKeyShortcutsAllowed ( false );
                            keyseq1->setModifierlessAllowed ( true );
                            keyseq2->setMultiKeyShortcutsAllowed ( false );
                            keyseq2->setModifierlessAllowed ( true );
                            hbox->addWidget(keyseq1);
                            hbox->addWidget(keyseq2);
                            widget->setLayout(hbox);

                            inputWidget = widget;
                            argument = hbox;
                            connect ( keyseq1, SIGNAL ( keySequenceChanged ( QKeySequence ) ), this, SIGNAL ( changed() ) );
                            connect ( keyseq2, SIGNAL ( keySequenceChanged ( QKeySequence ) ), this, SIGNAL ( changed() ) );
                        }

                        break;

                    case T_File:

                    case T_Char:

                    case T_String:
                        {
                            KLineEdit* lineEdit = new KLineEdit ( this );
                            inputWidget = lineEdit;
                            argument = inputWidget;

                            connect ( lineEdit, SIGNAL ( textChanged ( QString ) ), this, SIGNAL ( changed() ) );
                        }

                        break;

                    case T_I18NString:
                        inputWidget = NULL;
                        argument = NULL;
                        break;
                    }

                    if ( inputWidget )
                    {
                        QLabel* label = new QLabel ( s, this );
                        formLayout->addRow ( label, inputWidget );
                        ConfigBindValue ( cfile, cgdesc->groupName, codesc->optionName, NULL, SyncFilterFunc, argument );
                    }
                }

                scrollarea->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

                scrollarea->setMinimumWidth ( form->sizeHint().width() + 20 );
                mainLayout->addWidget ( scrollarea );

                m_ui->tabWidget->addTab ( main, QString::fromUtf8 ( dgettext ( m_cfdesc->domain, cgdesc->groupName ) ) );
            }

            ConfigBindSync ( &gconfig );
        }

    }


    void FcitxConfigPage::setupSubConfigUi()
    {
        QStringList keys = m_parser->getSubConfigKeys();
        if (keys.length() != 0)
        {
            QWidget* main = new QWidget ( this );
            QVBoxLayout* mainLayout = new QVBoxLayout;
            main->setLayout ( mainLayout );

            QScrollArea *scrollarea = new QScrollArea;
            scrollarea->setFrameStyle ( QFrame::NoFrame );
            scrollarea->setWidgetResizable ( true );

            QWidget* form = new QWidget;
            QFormLayout* formLayout = new QFormLayout;
            scrollarea->setWidget ( form );
            form->setLayout ( formLayout );

            Q_FOREACH(const QString& key, keys)
            {
                FcitxSubConfig* subconfig = m_parser->getSubConfig(key);
                formLayout->addRow(
                    QString::fromUtf8(
                        dgettext(m_parser->domain().toUtf8().data(),
                                 subconfig->name().toUtf8().data()
                                )
                        ),
                    new FcitxSubConfigWidget(subconfig, this));
            }

            scrollarea->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

            scrollarea->setMinimumWidth ( form->sizeHint().width() + 20 );
            mainLayout->addWidget ( scrollarea );
            m_ui->tabWidget->addTab(main, i18n("Other"));
        }
    }

    void SyncFilterFunc ( GenericConfig* gconfig, ConfigGroup *group, ConfigOption *option, void *value, ConfigSync sync, void *arg )
    {
        Q_UNUSED ( gconfig );
        Q_UNUSED ( group );
        Q_UNUSED ( value );
        ConfigOptionDesc *codesc = option->optionDesc;

        if ( !codesc )
            return;

        if ( sync == Raw2Value )
        {
            switch ( codesc->type )
            {

            case T_I18NString:
                break;

            case T_Integer:
                {
                    int value = atoi ( option->rawValue );
                    QSpinBox* spinbox = static_cast<QSpinBox*> ( arg );
                    spinbox->setValue ( value );
                }

                break;

            case T_Color:
                {
                    int r = 0,g = 0,b = 0;
                    sscanf ( option->rawValue, "%d %d %d",&r,&g,&b );
                    r = RoundColor ( r );
                    g = RoundColor ( g );
                    b = RoundColor ( b );
                    QColor color ( r, g, b );
                    KColorButton* colorButton = static_cast<KColorButton*> ( arg );
                    colorButton->setColor ( color );
                }

                break;

            case T_Boolean:
                {
                    bool bl;

                    if ( strcmp ( option->rawValue, "True" ) == 0 )
                        bl = true;
                    else
                        bl = false;

                    QCheckBox* checkBox = static_cast<QCheckBox*> ( arg );

                    checkBox->setChecked ( bl );
                }

                break;

            case T_Font:
                {
                    KFontComboBox *fontComboBox = static_cast<KFontComboBox*> ( arg );
                    QFont font ( QString::fromUtf8 ( option->rawValue ) );
                    fontComboBox->setCurrentFont ( font );
                }

                break;

            case T_Enum:
                {
                    ConfigEnum* cenum = &codesc->configEnum;
                    int index = 0, i;

                    for ( i = 0; i< cenum->enumCount; i++ )
                    {
                        if ( strcmp ( cenum->enumDesc[i], option->rawValue ) == 0 )
                        {
                            index = i;
                        }
                    }

                    KComboBox* combobox = static_cast<KComboBox*> ( arg );

                    combobox->setCurrentIndex ( index );
                }

                break;

            case T_Hotkey:
                {
                    HOTKEYS hotkey[2];

                    SetHotKey ( option->rawValue, hotkey );

                    QHBoxLayout* hbox = static_cast<QHBoxLayout*>(arg);
                    KKeySequenceWidget* keyseq[2];
                    keyseq[0] = static_cast<KKeySequenceWidget*>(hbox->itemAt(0)->widget());
                    keyseq[1] = static_cast<KKeySequenceWidget*>(hbox->itemAt(1)->widget());

                    int j;
                    for (j = 0; j < 2; j ++)
                    {
                        keyseq[j]->setKeySequence(HotkeyToKeySequence(&hotkey[j]));
                        if (hotkey[j].desc)
                            free(hotkey[j].desc);
                    }
                }

                break;

            case T_File:

            case T_Char:

            case T_String:
                {
                    KLineEdit* lineEdit = static_cast<KLineEdit*> ( arg );
                    lineEdit->setText ( QString::fromUtf8 ( option->rawValue ) );
                }

                break;
            }
        }
        else
        {
            if ( codesc->type != T_I18NString && option->rawValue )
            {
                free ( option->rawValue );
                option->rawValue = NULL;
            }

            switch ( codesc->type )
            {

            case T_I18NString:
                break;

            case T_Integer:
                {
                    int value;
                    QSpinBox* spinbox = static_cast<QSpinBox*> ( arg );
                    value = spinbox->value();
                    asprintf ( &option->rawValue, "%d", value );
                }

                break;

            case T_Color:
                {
                    int r = 0,g = 0,b = 0;
                    QColor color;
                    KColorButton* colorButton = static_cast<KColorButton*> ( arg );
                    color = colorButton->color();
                    r = color.red();
                    g = color.green();
                    b = color.blue();
                    r = RoundColor ( r );
                    g = RoundColor ( g );
                    b = RoundColor ( b );
                    asprintf ( &option->rawValue, "%d %d %d",r,g,b );
                }

                break;

            case T_Boolean:
                {
                    QCheckBox* checkBox = static_cast<QCheckBox*> ( arg );
                    bool bl;
                    bl = checkBox->isChecked();

                    if ( bl )
                        option->rawValue = strdup ( "True" );
                    else
                        option->rawValue = strdup ( "False" );
                }

                break;

            case T_Font:
                {
                    KFontComboBox *fontComboBox = static_cast<KFontComboBox*> ( arg );
                    const QFont& font = fontComboBox->currentFont();
                    option->rawValue = strdup ( font.family().toUtf8().data() );
                }

                break;

            case T_Enum:
                {
                    KComboBox* combobox = static_cast<KComboBox*> ( arg );
                    ConfigEnum* cenum = &codesc->configEnum;
                    int index = 0;
                    index = combobox->currentIndex();
                    option->rawValue = strdup ( cenum->enumDesc[index] );
                }

                break;

            case T_Hotkey:
                {
                    QHBoxLayout* hbox = static_cast<QHBoxLayout*>(arg);
                    KKeySequenceWidget* keyseq[2];
                    keyseq[0] = static_cast<KKeySequenceWidget*>(hbox->itemAt(0)->widget());
                    keyseq[1] = static_cast<KKeySequenceWidget*>(hbox->itemAt(1)->widget());
                    char *strkey[2] = { NULL, NULL };
                    int j = 0, k = 0;

                    HOTKEYS hotkey[2];

                    for (j = 0; j < 2 ; j ++)
                    {
                        if (KeySequenceToHotkey(keyseq[j]->keySequence(), &hotkey[j]))
                        {
                            strkey[k] = GetKeyString(hotkey[j].sym, hotkey[j].state);
                            if (strkey[k])
                                k ++;
                        }
                    }
                    if (strkey[1])
                        asprintf(&option->rawValue, "%s %s", strkey[0], strkey[1]);
                    else if (strkey[0])
                    {
                        option->rawValue = strdup(strkey[0]);
                    }
                    else
                        option->rawValue = strdup("");

                    for (j = 0 ; j < k ; j ++)
                        free(strkey[j]);
                }

                break;

            case T_File:

            case T_Char:

            case T_String:
                {
                    KLineEdit* lineEdit = static_cast<KLineEdit*> ( arg );
                    option->rawValue = strdup ( lineEdit->text().toUtf8().data() );
                }

                break;
            }
        }
    }


    bool
    KeySequenceToHotkey(const QKeySequence& keyseq, HOTKEYS* hotkey)
    {
        if (keyseq.count() != 1)
            return false;
        int key = keyseq[0];
        hotkey->sym = (FcitxKeySym) keyQtToSym(key);
        hotkey->state = KEY_NONE;

        if (key & Qt::CTRL)
            hotkey->state |= KEY_CTRL_COMP;

        if (key & Qt::ALT)
            hotkey->state |= KEY_ALT_COMP;

        if (key & Qt::SHIFT)
            hotkey->state |= KEY_SHIFT_COMP;

        return true;
    }

    QKeySequence
    HotkeyToKeySequence(HOTKEYS* hotkey)
    {
        int state = hotkey->state;
        FcitxKeySym keyval = hotkey->sym;

        Qt::KeyboardModifiers qstate = Qt::NoModifier;

        int count = 1;
        if(state & KEY_ALT_COMP)
        {
            qstate |= Qt::AltModifier;
            count ++;
        }

        if(state & KEY_SHIFT_COMP)
        {
            qstate |= Qt::ShiftModifier;
            count ++;
        }

        if(state & KEY_CTRL_COMP)
        {
            qstate |= Qt::ControlModifier;
            count ++;
        }

        int key;
        symToKeyQt((uint) keyval, key);

        return QKeySequence(key | qstate);
    }

}

#include "moc_FcitxConfigPage.cpp"
