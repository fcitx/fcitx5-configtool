#include <QDebug>
#include <QVBoxLayout>
#include <QListView>
#include <QStandardItemModel>

#include <KDialog>
#include <KPushButton>
#include <KRun>
#include "ConfigDescManager.h"
#include "FcitxConfigPage.h"
#include "FcitxSubConfigParser.h"
#include "FcitxSubConfigWidget.h"
#include "FcitxSubConfigWidget_p.h"
#include <fcitx-config/xdg.h>

namespace Fcitx {

    FcitxConfigFile::FcitxConfigFile(const QString& path) :
        m_path(path)
    {
    }

    QString FcitxConfigFile::name()
    {
        return m_path;
    }

    const QString& FcitxConfigFile::path() const
    {
        return m_path;
    }

    FcitxConfigFileItemModel::FcitxConfigFileItemModel(QObject* parent):
        QAbstractListModel(parent)
    {
    }

    FcitxConfigFileItemModel::~FcitxConfigFileItemModel()
    {
        Q_FOREACH(FcitxConfigFile* file, m_files)
        {
            delete file;
        }
    }

    QModelIndex FcitxConfigFileItemModel::index(int row, int column, const QModelIndex& parent) const
    {
        Q_UNUSED ( parent );

        return createIndex ( row, column, ( row < m_files.count() ) ? ( void* ) m_files.at ( row ) : 0 );
    }

    int FcitxConfigFileItemModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        return m_files.size();
    }

    QVariant FcitxConfigFileItemModel::data(const QModelIndex& index, int role) const
    {
        if ( !index.isValid() || !index.internalPointer() )
        {
            return QVariant();
        }

        FcitxConfigFile *configfile = static_cast<FcitxConfigFile*> ( index.internalPointer() );

        if (role == Qt::DisplayRole)
            return configfile->name();

        return QVariant();
    }

    void FcitxConfigFileItemModel::addConfigFile(FcitxConfigFile* configfile)
    {
        beginInsertRows ( QModelIndex(), m_files.count(), m_files.count() );
        m_files << configfile;
        endInsertRows();
    }

    FcitxSubConfigWidget::FcitxSubConfigWidget ( FcitxSubConfig* subconfig, QWidget* parent ) :
        QWidget ( parent ), m_subConfig(subconfig)
    {
        switch(subconfig->type())
        {
            case SC_ConfigFile:
                {
                    QVBoxLayout* hbox = new QVBoxLayout;
                    this->setLayout(hbox);
                    m_listView = new QListView;
                    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
                    m_model = new FcitxConfigFileItemModel(this);
                    Q_FOREACH(const QString& file, subconfig->filelist().uniqueKeys())
                    {
                        m_model->addConfigFile(new FcitxConfigFile(file));
                    }
                    m_listView->setModel(m_model);
                    hbox->addWidget(m_listView);

                    KPushButton* pushButton = new KPushButton;
                    pushButton->setIcon(KIcon("configure"));
                    connect(pushButton, SIGNAL(clicked()), this, SLOT(OpenSubConfig()));
                    hbox->addWidget(pushButton);
                }
                break;
            case SC_NativeFile:
                {
                    QVBoxLayout* hbox = new QVBoxLayout;
                    this->setLayout(hbox);
                    KPushButton* pushButton = new KPushButton;
                    pushButton->setIcon(KIcon("document-open"));
                    connect(pushButton, SIGNAL(clicked()), this, SLOT(OpenNativeFile()));
                    hbox->addWidget(pushButton);
                }
                break;
            default:
                break;
        }
    }

    FcitxSubConfigWidget::~FcitxSubConfigWidget()
    {
        delete m_subConfig;
    }

    void FcitxSubConfigWidget::OpenSubConfig()
    {
        QItemSelectionModel* selectionModel = m_listView->selectionModel();
        QModelIndex ind = selectionModel->currentIndex();
        if (!ind.isValid())
            return;
        FcitxConfigFile* configfile = static_cast<FcitxConfigFile*>(ind.internalPointer());
        KDialog configDialog;
        ConfigDescManager manager;
        ConfigFileDesc* cfdesc = manager.GetConfigDesc(m_subConfig->configdesc());

        if ( cfdesc )
        {
            FcitxConfigPage* configPage = new FcitxConfigPage (
                &configDialog,
                cfdesc,
                "",
                configfile->path()
            );
            configDialog.setButtons ( KDialog::Ok | KDialog::Cancel | KDialog::Default );
            configDialog.setMainWidget ( configPage );
            connect ( &configDialog, SIGNAL ( buttonClicked ( KDialog::ButtonCode ) ), configPage, SLOT ( buttonClicked ( KDialog::ButtonCode ) ) );

            configDialog.exec();
        }
    }

    void FcitxSubConfigWidget::OpenNativeFile()
    {
        QMultiMap< QString, FcitxSubConfigPath >& filelist = m_subConfig->filelist();
        if (filelist.size() > 0)
        {
            KRun::runUrl(KUrl(filelist.begin()->path()), "text/plain", NULL);
        }
        else
        {
            char *newpath = NULL;
            FILE* fp = GetXDGFileUserWithPrefix("", m_subConfig->nativepath().toUtf8().data(), "w", &newpath);
            if (fp)
            {
                filelist.insert(m_subConfig->nativepath(), FcitxSubConfigPath("", QString(newpath)));
                fclose(fp);
                KRun::runUrl(KUrl(filelist.begin()->path()), "text/plain", NULL);
            }
            free(newpath);
        }
    }
}