#include "config.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QPluginLoader>
#include <QSessionManager>
#include <QtPlugin>
#include <fcitx-utils/i18n.h>

Q_IMPORT_PLUGIN(PinyinMigratorPlugin);
Q_IMPORT_PLUGIN(SkkMigratorPlugin);
Q_IMPORT_PLUGIN(KkcMigratorPlugin);
Q_IMPORT_PLUGIN(RimeMigratorPlugin);
Q_IMPORT_PLUGIN(GlobalConfigMigratorPlugin);
Q_IMPORT_PLUGIN(TableMigratorPlugin);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("fcitx5-migrator");
    app.setApplicationVersion(PROJECT_VERSION);
    app.setApplicationDisplayName(_("Fcitx 5 Migration Wizard"));
    app.setOrganizationDomain("fcitx.org");
    fcitx::registerFcitxQtDBusTypes();

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest,
                     disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest,
                     disableSessionManagement);

    fcitx::MainWindow window;
    window.show();
    return app.exec();
}
