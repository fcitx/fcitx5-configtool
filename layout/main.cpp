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

#include <KApplication>
#include <KCmdLineArgs>
#include <QMainWindow>
#include <KIcon>

#include "config.h"
#include "keyboardlayoutwidget.h"

int main(int argc, char* argv[])
{
    KCmdLineArgs::init(argc, argv,
                       "kbd-layout-viewer", "kcm_fcitx",
                       ki18n("Keyboard Layout viewer"),
                       VERSION_STRING_FULL,
                       ki18n("A general keyboard layout viewer")
                      );
    KCmdLineOptions options;
    options.add("g");
    options.add("group <group>", ki18n("Keyboard layout group (0-3)"));
    options.add("l");
    options.add("layout <layout>", ki18n("Keyboard layout"));
    options.add("v");
    options.add("variant <variant>", ki18n("Keyboard layout variant"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    int group = -1;
    QString variant, layout;
    if (args->isSet("group")) {
        group = args->getOption("group").toInt();
    }
    if (args->isSet("layout"))
        layout = args->getOption("layout");
    if (args->isSet("variant"))
        variant = args->getOption("variant");

    KApplication app;
    QMainWindow mainWindow;
    mainWindow.setWindowIcon(KIcon("input-keyboard"));
    mainWindow.setWindowTitle(i18n("Keyboard Layout viewer"));
    mainWindow.setMinimumSize(QSize(900,400));
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