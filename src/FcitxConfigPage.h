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

#ifndef FCITXCONFIGPAGE_H
#define FCITXCONFIGPAGE_H

// Qt
#include <QWidget>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>

// self
#include "ui_FcitxConfigPage.h"

class QStandardItemModel;
struct _FcitxConfigFileDesc;

class QTabWidget;

namespace Fcitx
{

class FcitxSubConfigParser;


class FcitxConfigPage : public QWidget
{
    Q_OBJECT

public:
    FcitxConfigPage(QWidget* parent, struct _FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subConfig = QString());
    virtual ~FcitxConfigPage();

Q_SIGNALS:
    void changed();

public Q_SLOTS:
    void buttonClicked(KDialog::ButtonCode);
    void load();
private:
    void setupConfigUi();
    void setupSubConfigUi();

    struct _FcitxConfigFileDesc* m_cfdesc;
    QString m_prefix;
    QString m_name;
    QTabWidget* m_tabWidget;
    Ui::FcitxConfigPage* m_ui;
    FcitxGenericConfig gconfig;
    FcitxSubConfigParser* m_parser;
};

}

#endif
