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
#include <QPointer>
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
#include "configdescmanager.h"
#include "configwidget.h"
#include "subconfigparser.h"
#include "subconfigwidget.h"
#include "subconfigwidget_p.h"
namespace Fcitx
{

ConfigFile::ConfigFile(const QString& path) :
    m_path(path)
{
}

QString ConfigFile::name()
{
    return m_path;
}

const QString& ConfigFile::path() const
{
    return m_path;
}

ConfigFileItemModel::ConfigFileItemModel(QObject* parent):
    QAbstractListModel(parent)
{
}

ConfigFileItemModel::~ConfigFileItemModel()
{
    Q_FOREACH(ConfigFile * file, m_files) {
        delete file;
    }
}

QModelIndex ConfigFileItemModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, (row < m_files.count()) ? (void*) m_files.at(row) : 0);
}

int ConfigFileItemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_files.size();
}

QVariant ConfigFileItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return QVariant();
    }

    ConfigFile *configfile = static_cast<ConfigFile*>(index.internalPointer());

    if (role == Qt::DisplayRole)
        return configfile->name();

    return QVariant();
}

void ConfigFileItemModel::addConfigFile(ConfigFile* configfile)
{
    beginInsertRows(QModelIndex(), m_files.count(), m_files.count());
    m_files << configfile;
    endInsertRows();
}

SubConfigWidget::SubConfigWidget(SubConfig* subconfig, QWidget* parent) :
    QWidget(parent), m_subConfig(subconfig)
{
    switch (subconfig->type()) {
    case SC_ConfigFile: {
        QVBoxLayout* hbox = new QVBoxLayout;
        this->setLayout(hbox);
        m_listView = new QListView;
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_model = new ConfigFileItemModel(this);
        Q_FOREACH(const QString & file, subconfig->filelist()) {
            m_model->addConfigFile(new ConfigFile(file));
        }
        m_listView->setModel(m_model);
        hbox->addWidget(m_listView);

        KPushButton* pushButton = new KPushButton;
        pushButton->setIcon(KIcon("configure"));
        connect(pushButton, SIGNAL(clicked()), this, SLOT(openSubConfig()));
        hbox->addWidget(pushButton);
    }
    break;
    case SC_NativeFile: {
        QVBoxLayout* hbox = new QVBoxLayout;
        this->setLayout(hbox);
        KPushButton* pushButton = new KPushButton;
        pushButton->setIcon(KIcon("document-open"));
        connect(pushButton, SIGNAL(clicked()), this, SLOT(openNativeFile()));
        hbox->addWidget(pushButton);
    }
    case SC_Program: {
        QVBoxLayout* hbox = new QVBoxLayout;
        this->setLayout(hbox);
        KPushButton* pushButton = new KPushButton;
        pushButton->setIcon(KIcon("system-run"));
        qDebug() << subconfig->program();
        if (subconfig->program().isNull())
            pushButton->setEnabled(false);
        else
            connect(pushButton, SIGNAL(clicked()), this, SLOT(openProgram()));
        hbox->addWidget(pushButton);
    }
    break;
    default:
        break;
    }
}

SubConfigWidget::~SubConfigWidget()
{
    delete m_subConfig;
}

void SubConfigWidget::openSubConfig()
{
    QItemSelectionModel* selectionModel = m_listView->selectionModel();
    QModelIndex ind = selectionModel->currentIndex();
    if (!ind.isValid())
        return;
    ConfigFile* configfile = static_cast<ConfigFile*>(ind.internalPointer());
    FcitxConfigFileDesc* cfdesc = ConfigDescManager::instance()->GetConfigDesc(m_subConfig->configdesc());

    if (cfdesc) {
        QPointer<KDialog> configDialog(ConfigWidget::configDialog(
            NULL,
            cfdesc,
            "",
            configfile->path()
        ));

        configDialog->exec();
        delete configDialog;
    }
}

void SubConfigWidget::openNativeFile()
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
        KRun::runUrl(KUrl(newpath), m_subConfig->mimetype().isEmpty() ? "text/plain" : m_subConfig->mimetype(), NULL);
        free(newpath);
    }
}

void SubConfigWidget::openProgram()
{
    KRun::runCommand(m_subConfig->program(), NULL);
}

}
