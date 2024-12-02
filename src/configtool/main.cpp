/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "config.h"
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QSessionManager>

#ifdef CONFIG_QT_UNIQUE_APP_SUPPORT
#include <KDBusService>
#include <KWindowSystem>
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("fcitx5-config-qt");
    app.setApplicationVersion(PROJECT_VERSION);
    app.setApplicationDisplayName(_("Fcitx Configuration"));
    app.setOrganizationDomain("fcitx.org");
    fcitx::registerFcitxQtDBusTypes();

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest,
                     disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest,
                     disableSessionManagement);

    fcitx::kcm::MainWindow mainWindow;

#ifdef CONFIG_QT_UNIQUE_APP_SUPPORT
    KDBusService dbusService(KDBusService::Unique);
    QObject::connect(
        &dbusService, &KDBusService::activateRequested, &mainWindow,
        [&mainWindow](const QStringList & /*args*/,
                      const QString & /*workingDir*/) {
            mainWindow.show();
            KWindowSystem::updateStartupId(mainWindow.windowHandle());
            mainWindow.raise();
            KWindowSystem::activateWindow(mainWindow.windowHandle());
        });
#endif

    mainWindow.show();

    return app.exec();
}
