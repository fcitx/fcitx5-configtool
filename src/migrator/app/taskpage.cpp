/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "taskpage.h"
#include "dbuscaller.h"
#include "dbuswatcher.h"
#include "mainwindow.h"
#include "migrator.h"
#include "processrunner.h"
#include <algorithm>
#include <fcitx-utils/i18n.h>

namespace fcitx {

class TaskModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit TaskModel(const MigratorFactory *parent) : factory_(parent) {}

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!checkIndex(index)) {
            return QVariant();
        }
        switch (role) {
        case Qt::DisplayRole:
            return migrators_[index.row()]->name();
        case Qt::WhatsThisRole:
            return migrators_[index.row()]->description();
        case Qt::CheckStateRole:
            if (migratorStatesCache_[index.row()]) {
                return selected_.contains(index) ? Qt::Checked : Qt::Unchecked;
            }
            break;
        default:
            break;
        }
        return {};
    }

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role) override {
        if (role != Qt::CheckStateRole || !checkIndex(index)) {
            return false;
        }

        bool newValue = (value == Qt::Checked);
        if (newValue == selected_.contains(index)) {
            return false;
        }
        if (newValue) {
            selected_.insert(index);
        } else {
            selected_.remove(index);
        }
        Q_EMIT dataChanged(index, index);
        Q_EMIT selectedChanged();
        return true;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractListModel::flags(index);
        if (!checkIndex(index)) {
            return flags;
        }

        flags.setFlag(Qt::ItemIsUserCheckable,
                      migratorStatesCache_[index.row()]);
        flags.setFlag(Qt::ItemIsEnabled, migratorStatesCache_[index.row()]);
        return flags;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return migrators_.size();
    }

    void refresh(const QSet<QString> &availableAddons) {
        beginResetModel();
        migrators_ = factory_->list(availableAddons);
        migratorStatesCache_.resize(migrators_.size());
        int idx = 0;
        int nAvailMigrators = 0;
        for (const auto &migrator : migrators_) {
            migratorStatesCache_[idx] = migrator->check();
            if (migratorStatesCache_[idx]) {
                nAvailMigrators++;
            }
            idx++;
        }
        selected_.clear();
        setAvailableSize(nAvailMigrators);
        endResetModel();
        Q_EMIT selectedChanged();
    }

    void selectAll() {
        if (migrators_.empty()) {
            return;
        }
        for (size_t i = 0; i < migrators_.size(); i++) {
            if (migratorStatesCache_[i]) {
                selected_.insert(createIndex(i, 0));
            }
        }
        Q_EMIT dataChanged(index(0, 0), index(migrators_.size() - 1));
        Q_EMIT selectedChanged();
    }

    void clearSelection() {
        selected_.clear();
        Q_EMIT dataChanged(index(0, 0), index(migrators_.size() - 1));
        Q_EMIT selectedChanged();
    }

    bool allSelected() const {
        return availableSize_ != 0 &&
               availableSize_ == selected_.size();
    }

    bool someSelected() const { return !selected_.isEmpty(); }

    std::vector<Migrator *> selectedMigrators() {
        std::vector<Migrator *> result;
        for (const QPersistentModelIndex &index : selected_) {
            result.push_back(migrators_[index.row()].get());
        }
        return result;
    }

    void setAvailableSize(int n) {
        if (n == availableSize_) {
            return;
        }
        availableSize_ = n;
        Q_EMIT availableSizeChanged(availableSize_);
    }

Q_SIGNALS:
    void selectedChanged();
    void availableSizeChanged(int availableSize);

private:
    const MigratorFactory *factory_;
    std::vector<std::unique_ptr<Migrator>> migrators_;
    std::vector<bool> migratorStatesCache_;
    int availableSize_ = 0;
    QSet<QPersistentModelIndex> selected_;
};

TaskPage::TaskPage(MainWindow *parent)
    : QWizardPage(parent), parent_(parent),
      model_(new TaskModel(parent_->factory())) {
    setupUi(this);
    setCommitPage(true);
    taskView->setModel(model_);
    fcitxNotRunningMessage->setVisible(!parent_->dbus()->available());

    connect(taskView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current) {
                if (current.isValid()) {
                    descriptionLabel->setText(
                        model_->data(current, Qt::WhatsThisRole).toString());
                } else {
                    descriptionLabel->setText(
                        _("Click on an item for more details."));
                }
            });
    connect(model_, &TaskModel::selectedChanged, this, [this]() {
        selectAllBox->setChecked(model_->allSelected());
        Q_EMIT completeChanged();
    });
    auto availableSizeChanged = [this](int availMigrators) {
        if (availMigrators) {
            selectAllBox->show();
            descriptionLabel->setText(_("Click on an item for more details."));
        } else {
            selectAllBox->hide();
            descriptionLabel->setText(_("No available migrators."));
        }
    };
    connect(model_, &TaskModel::availableSizeChanged, this,
            availableSizeChanged);
    connect(selectAllBox, &QCheckBox::clicked, this, [this](bool checked) {
        if (checked) {
            model_->selectAll();
        } else {
            model_->clearSelection();
        }
    });
    availableSizeChanged(0);
}

void TaskPage::availabilityChanged(bool avail) {
    fcitxNotRunningMessage->setVisible(!avail);
    if (avail) {
        auto call = parent_->dbus()->controller()->GetAddonsV2();
        auto *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();
                    QDBusPendingReply<FcitxQtAddonInfoV2List> reply = *watcher;
                    if (!reply.isValid()) {
                        return;
                    }
                    QSet<QString> addons;
                    for (const auto &addon : reply.value()) {
                        if (addon.enabled()) {
                            addons << addon.uniqueName();
                        }
                    }
                    model_->refresh(addons);
                });
    }
}

bool TaskPage::isComplete() const {
    return parent_->dbus()->available() && model_->someSelected();
}

void TaskPage::initializePage() {
    setButtonText(QWizard::CommitButton, _("&Next >"));

    availabilityChanged(parent_->dbus()->available());

    connect(parent_->dbus(), &kcm::DBusProvider::availabilityChanged, this,
            &QWizardPage::completeChanged);
    connect(parent_->dbus(), &kcm::DBusProvider::availabilityChanged, this,
            &TaskPage::availabilityChanged);
}

bool TaskPage::validatePage() {
    disconnect(parent_->dbus(), nullptr, this, nullptr);
    return true;
}

Pipeline *TaskPage::createPipeline() {
    Pipeline *pipeline = new Pipeline;
    auto migrators = model_->selectedMigrators();
    bool hasOfflineJob = std::any_of(
        migrators.begin(), migrators.end(),
        [](const Migrator *migrator) { return migrator->hasOfflineJob(); });

    if (hasOfflineJob) {
        pipeline->addJob(new DBusCaller(
            [this]() -> QDBusPendingCallWatcher * {
                if (parent_->dbus()->available()) {
                    return new QDBusPendingCallWatcher(
                        parent_->dbus()->controller()->Exit());
                } else {
                    return nullptr;
                }
            },
            _("Asking Fcitx to exit..."), _("Fcitx Exited.")));
        pipeline->addJob(new DBusWatcher(
            "org.fcitx.Fcitx5", _("Checking if Fcitx is still running..."),
            _("Fcitx is fully exited."), false));
        for (auto *migrator : migrators) {
            migrator->addOfflineJob(pipeline);
        }

        auto *startFcitxJob = new ProcessRunner("fcitx5", {"-d"}, {});
        startFcitxJob->setStartMessage("Restarting Fcitx 5...");
        startFcitxJob->setFinishMessage("Fcitx 5 is restarted.");
        pipeline->addJob(startFcitxJob);
    }
    pipeline->addJob(new DBusWatcher("org.fcitx.Fcitx5",
                                     _("Checking if Fcitx is running..."),
                                     _("Fcitx is started."), true));
    for (auto *migrator : migrators) {
        migrator->addOnlineJob(pipeline);
    }
    return pipeline;
}

} // namespace fcitx

#include "taskpage.moc"
