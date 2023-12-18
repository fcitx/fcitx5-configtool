/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "config.h"
#include "mainwindow.h"
#include "logging.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("fcitx5-config-qt");
    app.setApplicationVersion(PROJECT_VERSION);
    app.setApplicationDisplayName(_("Fcitx Configuration"));
    app.setOrganizationDomain("fcitx.org");
    fcitx::registerFcitxQtDBusTypes();

    const QString &localName = QLocale::system().name();
    const QString &transPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QStringList qtTransFiles = { transPath + QString("/qt_") + localName,
                               transPath + QString("/qtbase_") + localName };
    for (const QString &filename : qtTransFiles) {
        QTranslator *translator = new QTranslator;
        bool ret = translator->load(filename);
        qCDebug(KCM_FCITX5) <<  "Loading translator" << filename;

        if (!ret) {
            delete translator;
            continue;
        }

        app.installTranslator(translator);
    }

    fcitx::kcm::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
