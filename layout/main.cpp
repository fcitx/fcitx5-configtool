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

#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QX11Info>

#include <KLocalizedString>

#include "config.h"
#include "keyboardlayoutwidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QLatin1String("kbd-layout-viewer"));
    app.setApplicationVersion(QLatin1String(VERSION_STRING_FULL));

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("A general keyboard layout viewer"));
    parser.addHelpOption();
    parser.addOptions({
        {{"g", "group"},
            i18n("Keyboard layout <group> (0-3)"),
            i18n("group")},
        {{"l", "layout"},
            i18n("Keyboard <layout>"),
            i18n("layout")},
        {{"v", "variant"},
            i18n("Keyboard layout <variant>"),
            i18n("variant")}
    });

    parser.process(app);

    int group = -1;
    QString variant, layout;
    if (parser.isSet("group")) {
        group = parser.value("group").toInt();
    }
    if (parser.isSet("layout")) {
        layout = parser.value("layout");
    }
    if (parser.isSet("variant")) {
        variant = parser.value("variant");
    }

    if (!QX11Info::isPlatformX11()) {
        qFatal("Only X11 is supported");
        return 1;
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    QMainWindow mainWindow;
    mainWindow.setWindowIcon(QIcon::fromTheme("input-keyboard"));
    mainWindow.setWindowTitle(i18n("Keyboard Layout viewer"));
    // mainWindow.setMinimumSize(QSize(900,400));
    KeyboardLayoutWidget widget;
    if (group > 0 || layout.isNull()) {
        if (group < 0)
            group = 0;
        widget.setGroup(group);
    }
    else if (!layout.isNull()) {
        widget.setKeyboardLayout(layout, variant);
    }

    mainWindow.setCentralWidget(&widget);
    mainWindow.show();

    return app.exec();
}
