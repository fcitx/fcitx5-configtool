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
#include <QDialog>
#include <KRun>
#include <KMessageBox>
#include <KLocalizedString>

#include <kio_version.h>

// Fcitx
#include <fcitx-config/xdg.h>

// self
#include "global.h"
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
        setLayout(hbox);
        m_listView = new QListView;
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_model = new ConfigFileItemModel(this);
        Q_FOREACH(const QString & file, subconfig->fileList()) {
            m_model->addConfigFile(new ConfigFile(file));
        }
        m_listView->setModel(m_model);
        hbox->addWidget(m_listView);

        QPushButton* pushButton = new QPushButton;
        pushButton->setIcon(QIcon::fromTheme("configure"));
        connect(pushButton, SIGNAL(clicked()), this, SLOT(openSubConfig()));
        hbox->addWidget(pushButton);
    }
    break;
    case SC_NativeFile: {
        QVBoxLayout* hbox = new QVBoxLayout;
        setLayout(hbox);
        QPushButton* pushButton = new QPushButton;
        pushButton->setIcon(QIcon::fromTheme("document-open"));
        connect(pushButton, SIGNAL(clicked()), this, SLOT(openNativeFile()));
        hbox->addWidget(pushButton);
    }
    break;
    case SC_Program: {
        QVBoxLayout* hbox = new QVBoxLayout;
        setLayout(hbox);
        QPushButton* pushButton = new QPushButton;
        pushButton->setIcon(QIcon::fromTheme("system-run"));
        if (subconfig->program().isNull())
            pushButton->setEnabled(false);
        else
            connect(pushButton, SIGNAL(clicked()), this, SLOT(openProgram()));
        hbox->addWidget(pushButton);
    }
    break;
    case SC_Plugin: {
        QVBoxLayout* hbox = new QVBoxLayout;
        setLayout(hbox);
        QPushButton* pushButton = new QPushButton;
        pushButton->setIcon(QIcon::fromTheme("configure"));
        connect(pushButton, SIGNAL(clicked()), this, SLOT(openPlugin()));
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
    FcitxConfigFileDesc* cfdesc = Global::instance()->GetConfigDesc(m_subConfig->configdesc());

    if (cfdesc) {
        QPointer<QDialog> configDialog(ConfigWidget::configDialog(
            NULL,
            cfdesc,
            "",
            configfile->path()
        ));

        configDialog->exec();
        delete configDialog;
    }
}

bool SubConfigWidget::launchGuiWrapper(const QString &path) {
    QString wrapper = Global::instance()->testWrapper(path);

    if (!wrapper.isEmpty()) {
        QStringList args;
        args << path;
        return QProcess::startDetached(wrapper, args);
    }
    return false;
}

void SubConfigWidget::openPlugin()
{
    launchGuiWrapper(m_subConfig->nativepath());
}

void SubConfigWidget::openNativeFile()
{
    char *newpath = NULL;

    if (launchGuiWrapper(m_subConfig->nativepath())) {
        return;
    }

    /* this configuration file doesn't have user version */
    if (m_subConfig->userFileList().size() == 0) {
        /* still if system version doesn't exit either, let's create an empty text file for user */
        if (m_subConfig->fileList().size() == 0) {
            FILE* fp = FcitxXDGGetFileUserWithPrefix("", m_subConfig->nativepath().toLocal8Bit().constData(), "w", &newpath);
            if (fp) {
                fclose(fp);
                m_subConfig->updateFileList();
            }
        }
        else {
            switch(KMessageBox::questionYesNoCancel(
                NULL,
                i18n("User config doesn't exisits, do you want to open system file or copy system file to user file?"),
                i18n("What to do"),
                KGuiItem(i18n("Copy")),
                KGuiItem(i18n("View system"))))
            {
                case KMessageBox::Yes:
                    {
                        char* src = NULL;
                        FILE* fp = FcitxXDGGetFileWithPrefix("", m_subConfig->fileList().begin()->toLocal8Bit().constData(), "r", &src);
                        if (fp)
                            fclose(fp);
                        FcitxXDGGetFileUserWithPrefix("", m_subConfig->nativepath().toLocal8Bit().constData(), NULL, &newpath);
                        QFile file(src);
                        free(src);
                        if (!file.copy(newpath)) {
                            KMessageBox::error(NULL, i18n("Copy failed"), i18n("Copy failed"));
                        }
                        m_subConfig->updateFileList();
                    }
                    break;
                case KMessageBox::No:
                    {
                        FILE* fp = FcitxXDGGetFileWithPrefix("", m_subConfig->fileList().begin()->toLocal8Bit().constData(), "r", &newpath);
                        if (fp)
                            fclose(fp);
                    }
                   break;
                default:
                    return;
            }
        }
    }
    else {
        FILE* fp = FcitxXDGGetFileWithPrefix("", m_subConfig->userFileList().begin()->toLocal8Bit().constData(), "r", &newpath);
        if (fp)
            fclose(fp);
    }

    if (newpath) {
#if KIO_VERSION < QT_VERSION_CHECK(5, 31, 0)
        KRun::runUrl(QUrl(newpath), m_subConfig->mimetype().isEmpty() ? "text/plain" : m_subConfig->mimetype(), NULL);
#else
        KRun::runUrl(QUrl(newpath), m_subConfig->mimetype().isEmpty() ? "text/plain" : m_subConfig->mimetype(), NULL, KRun::RunFlags());
#endif
        free(newpath);
    }
}

void SubConfigWidget::openProgram()
{
    KRun::runCommand(m_subConfig->program(), NULL);
}

}
