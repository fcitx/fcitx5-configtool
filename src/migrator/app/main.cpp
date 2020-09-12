#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QPluginLoader>
#include <QtPlugin>

Q_IMPORT_PLUGIN(PinyinMigratorPlugin);
Q_IMPORT_PLUGIN(SkkMigratorPlugin);
Q_IMPORT_PLUGIN(KkcMigratorPlugin);
Q_IMPORT_PLUGIN(RimeMigratorPlugin);
Q_IMPORT_PLUGIN(GlobalConfigMigratorPlugin);
Q_IMPORT_PLUGIN(TableMigratorPlugin);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    fcitx::MainWindow window;
    window.show();
    return app.exec();
}
