/***************************************************************************
 *   Copyright (C) 2012~2012 by CSSlayer                                   *
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

#ifndef FCITX_UI_PAGE_H
#define FCITX_UI_PAGE_H

// Qt
#include <QString>
#include <QWidget>
#include <QDBusConnection>

// self
#include "inputmethodproxy.h"

class QLabel;
class QVBoxLayout;
namespace Fcitx
{

class ConfigWidget;

class Module;
class UIPage : public QWidget
{
    Q_OBJECT
public:
    UIPage(Module* parent = 0);
    virtual ~UIPage();
Q_SIGNALS:
    void changed();
public Q_SLOTS:
    void save();
    void load();
    void getUIFinished(QDBusPendingCallWatcher* watcher);
private:
    Module* m_module;
    InputMethodProxy* m_proxy;
    QVBoxLayout* m_layout;
    QLabel* m_label;
    ConfigWidget* m_widget;
};
}

#endif

