/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "migratorfactory.h"
#include "migratorfactory_p.h"
#include "migratorfactoryplugin.h"
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <QSet>
#include <QVariant>
#include <QVector>
#include <fcitx-utils/standardpath.h>

namespace fcitx {

MigratorFactoryPrivate::MigratorFactoryPrivate(MigratorFactory *factory)
    : QObject(factory), q_ptr(factory) {}

MigratorFactoryPrivate::~MigratorFactoryPrivate() {}

void MigratorFactoryPrivate::scan() {
    for (const auto &staticPlugin : QPluginLoader::staticPlugins()) {
        auto metadata = staticPlugin.metaData();
        if (metadata.value("IID") !=
            QLatin1String(FcitxMigratorFactoryInterface_iid)) {
            continue;
        }
        auto pluginMetadata = metadata.value("MetaData").toObject();
        auto addon = metadata.value("addon").toVariant().toString();
        if (auto plugin = qobject_cast<FcitxMigratorFactoryPlugin *>(
                staticPlugin.instance())) {
            plugins_.emplace_back(plugin, addon);
        }
    }

    StandardPath::global().scanFiles(
        StandardPath::Type::Addon, "qt5",
        [this](const std::string &path, const std::string &dirPath, bool user) {
            do {
                if (user) {
                    break;
                }

                QDir dir(QString::fromLocal8Bit(dirPath.c_str()));
                QFileInfo fi(
                    dir.filePath(QString::fromLocal8Bit(path.c_str())));

                QString filePath = fi.filePath(); // file name with path
                QString fileName = fi.fileName(); // just file name

                if (!QLibrary::isLibrary(filePath)) {
                    break;
                }

                QPluginLoader *loader = new QPluginLoader(filePath, this);
                if (loader->metaData().value("IID") !=
                    QLatin1String(FcitxMigratorFactoryInterface_iid)) {
                    delete loader;
                    continue;
                }
                auto metadata = loader->metaData().value("MetaData").toObject();
                auto addon = metadata.value("addon").toVariant().toString();
                if (auto plugin = qobject_cast<FcitxMigratorFactoryPlugin *>(
                        loader->instance())) {
                    plugins_.emplace_back(plugin, addon);
                } else {
                    delete loader;
                }
            } while (0);
            return true;
        });
}

MigratorFactory::MigratorFactory(QObject *parent)
    : QObject(parent), d_ptr(new MigratorFactoryPrivate(this)) {
    Q_D(MigratorFactory);
    d->scan();
}

MigratorFactory::~MigratorFactory() {}

std::vector<std::unique_ptr<Migrator>>
MigratorFactory::list(const QSet<QString> &addons) const {
    Q_D(const MigratorFactory);
    std::vector<std::unique_ptr<Migrator>> result;
    for (const auto &[plugin, addon] : d->plugins_) {
        if (addon.isEmpty() || addons.contains(addon)) {
            if (auto migrator = plugin->create()) {
                result.emplace_back(migrator);
            }
        }
    }
    return result;
}

} // namespace fcitx
