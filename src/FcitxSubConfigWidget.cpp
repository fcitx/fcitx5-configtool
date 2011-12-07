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
#include <QDebug>
#include <QVBoxLayout>
#include <QListView>
#include <QStandardItemModel>

// KDE
#include <KDialog>
#include <KPushButton>
#include <KRun>

// Fcitx
#include <fcitx-config/xdg.h>

// self
#include "ConfigDescManager.h"
#include "FcitxConfigPage.h"
#include "FcitxSubConfigParser.h"
#include "FcitxSubConfigWidget.h"
#include "FcitxSubConfigWidget_p.h"
namespace Fcitx
{

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
    Q_FOREACH(FcitxConfigFile * file, m_files) {
        delete file;
    }
}

QModelIndex FcitxConfigFileItemModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, (row < m_files.count()) ? (void*) m_files.at(row) : 0);
}

int FcitxConfigFileItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_files.size();
}

QVariant FcitxConfigFileItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return QVariant();
    }

    FcitxConfigFile *configfile = static_cast<FcitxConfigFile*>(index.internalPointer());

    if (role == Qt::DisplayRole)
        return configfile->name();

    return QVariant();
}

void FcitxConfigFileItemModel::addConfigFile(FcitxConfigFile* configfile)
{
    beginInsertRows(QModelIndex(), m_files.count(), m_files.count());
    m_files << configfile;
    endInsertRows();
}

FcitxSubConfigWidget::FcitxSubConfigWidget(FcitxSubConfig* subconfig, QWidget* parent) :
    QWidget(parent), m_subConfig(subconfig)
{
    switch (subconfig->type()) {
    case SC_ConfigFile: {
        QVBoxLayout* hbox = new QVBoxLayout;
        this->setLayout(hbox);
        m_listView = new QListView;
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_model = new FcitxConfigFileItemModel(this);
        Q_FOREACH(const QString & file, subconfig->filelist()) {
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
    case SC_NativeFile: {
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
    FcitxConfigFileDesc* cfdesc = manager.GetConfigDesc(m_subConfig->configdesc());

    if (cfdesc) {
        FcitxConfigPage* configPage = new FcitxConfigPage(
            &configDialog,
            cfdesc,
            "",
            configfile->path()
        );
        configDialog.setWindowIcon(KIcon("fcitx"));
        configDialog.setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
        configDialog.setMainWidget(configPage);
        connect(&configDialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), configPage, SLOT(buttonClicked(KDialog::ButtonCode)));

        configDialog.exec();
    }
}

void FcitxSubConfigWidget::OpenNativeFile()
{
    QSet< QString >& filelist = m_subConfig->filelist();
    char *newpath = NULL;
    if (filelist.size() > 0) {
        FILE* fp = FcitxXDGGetFileWithPrefix("", filelist.begin()->toLocal8Bit().data(), "r", &newpath);
        if (fp)
            fclose(fp);
    } else {
        FILE* fp = FcitxXDGGetFileUserWithPrefix("", m_subConfig->nativepath().toLocal8Bit().data(), "w", &newpath);
        if (fp) {
            filelist.insert(m_subConfig->nativepath());
            fclose(fp);
        }
    }
    if (newpath) {
        KRun::runUrl(KUrl(newpath), "text/plain", NULL);
        free(newpath);
    }
}

}