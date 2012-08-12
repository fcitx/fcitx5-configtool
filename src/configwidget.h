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

// KDE
#include <KDialog>
#include <KColorButton>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx/addon.h>

class QCheckBox;
class QVBoxLayout;
class QStandardItemModel;
struct _FcitxConfigFileDesc;

class QTabWidget;

namespace Fcitx
{

class DummyConfig;

class ConfigDescManager;

class SubConfigParser;


class ColorButton : public KColorButton {
    Q_OBJECT
public:
    explicit ColorButton(QWidget* parent = 0) : KColorButton(parent) { }
public slots:
    void setColor(const QColor& color) {
        KColorButton::setColor(color);
    }
};

class ConfigWidget : public QWidget
{
    Q_OBJECT

    enum UIType {
        CW_NotSet = 0x4,
        CW_SimpleOnly = 0x1,
        CW_FullOnly = 0x2,
        CW_NoConfig = 0x0,
        CW_SimpleAndFull=0x3
    };

public:
    explicit ConfigWidget(struct _FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig = QString(), QWidget* parent = NULL);
    virtual ~ConfigWidget();

    static KDialog* configDialog(QWidget* parent, _FcitxConfigFileDesc* cfdesc, const QString& prefix, const QString& name, const QString& subconfig = QString());
    static KDialog* configDialog(QWidget* parent, FcitxAddon* addonEntry);

Q_SIGNALS:
    void changed();

public Q_SLOTS:
    void buttonClicked(KDialog::ButtonCode);
    void load();
private Q_SLOTS:
    void toggleSimpleFull();
private:
    void setupFullConfigUi();
    void setupSimpleConfigUi();
    void setupConfigUi();
    void createConfigOptionWidget(FcitxConfigGroupDesc* cgdesc, FcitxConfigOptionDesc* codesc, QString& s, QWidget*& inputWidget, void*& newarg);
    void checkCanUseSimple();

    struct _FcitxConfigFileDesc* m_cfdesc;
    QString m_prefix;
    QString m_name;
    QVBoxLayout* m_switchLayout;
    QWidget* m_simpleWidget;
    QWidget* m_fullWidget;
    QCheckBox* m_advanceCheckBox;
    DummyConfig* m_config;
    SubConfigParser* m_parser;
    UIType m_uitype;
    QMap<QString, void*> m_argMap;
};

}

#endif
