/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    fcitx::registerFcitxQtDBusTypes();

    fcitx::kcm::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
