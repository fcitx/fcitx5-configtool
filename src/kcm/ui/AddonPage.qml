/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: addonPage
    property bool needsSave: false
    property string reenableAddon

    title: i18n("Addons")

    function defaults() {
    }
    function load() {
        kcm.loadAddon();
        needsSave = false;
    }
    function save() {
        kcm.saveAddon();
        needsSave = false;
    }
    function showWarning() {
        dialog.open();
    }

    Binding {
        property: "filterText"
        target: kcm ? kcm.addonModel : null
        value: search.text
    }
    SaveWarningDialog {
        id: dialog
    }

    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: showOptionWarning
            Layout.fillWidth: true
            visible: showEnable.checked
            type: Kirigami.MessageType.Warning
            text: i18n("The feature of enabling/disabling addons is only intended for advanced users who understand the potential implication. Fcitx needs to be restarted to make the changes to enable/disable to take effect.")
        }
        Kirigami.InlineMessage {
            id: disableAddonWarning
            Layout.fillWidth: true
            showCloseButton: true
            type: Kirigami.MessageType.Warning

            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    icon.name: "edit-undo"
                    text: i18n("Re-Enable")

                    onTriggered: {
                        kcm.addonModel.sourceModel.enable(reenableAddon);
                        disableAddonWarning.visible = false;
                    }
                }
            ]
        }
        RowLayout {
            TextField {
                id: search
                Layout.fillWidth: true
                placeholderText: i18n("Search...")
            }
        }
    }
    view: ListView {
        model: kcm ? kcm.addonModel : null

        section {
            property: "categoryName"

            delegate: Kirigami.ListSectionHeader {
                label: section
            }
        }

        delegate: Kirigami.SwipeListItem {
            id: listItem
            contentItem: RowLayout {
                CheckBox {
                    id: itemChecked
                    Layout.alignment: Qt.AlignVCenter
                    Layout.leftMargin: Kirigami.Units.gridUnit
                    checked: model.enabled
                    visible: showEnable.checked

                    onClicked: {
                        model.enabled = !model.enabled;
                        if (!model.enabled) {
                            var dependencies = model.dependencies;
                            var optionalDependencies = model.optionalDependencies;
                            if (dependencies.length > 0 || optionalDependencies.length > 0) {
                                reenableAddon = model.uniqueName;
                                var sep = i18nc("Separator of a comma list", ", ");
                                var depWarning = "";
                                if (dependencies.length > 0) {
                                    var addonNames = [];
                                    for (var i = 0; i < dependencies.length; i++) {
                                        var name = kcm.addonModel.sourceModel.addonName(dependencies[i]);
                                        if (name) {
                                            addonNames.push(name);
                                        }
                                    }
                                    depWarning = depWarning.concat(i18n("- Disable %1\n", addonNames.join(sep)));
                                }
                                if (optionalDependencies.length > 0) {
                                    var addonNames = [];
                                    for (var i = 0; i < optionalDependencies.length; i++) {
                                        var name = kcm.addonModel.sourceModel.addonName(optionalDependencies[i]);
                                        if (name) {
                                            addonNames.push(name);
                                        }
                                    }
                                    depWarning = depWarning.concat(i18n("- Disable some features in %1\n", addonNames.join(sep)));
                                }
                                disableAddonWarning.text = i18n("Disabling %1 will also:\n%2\nAre you sure you want to disable it?", model.name, depWarning);
                                disableAddonWarning.visible = true;
                            }
                        }
                        needsSave = true;
                    }
                }
                ColumnLayout {
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        level: 5
                        text: model.enabled ? model.name : i18n("%1 (Disabled)", model.name)
                    }
                    Label {
                        opacity: listItem.hovered ? 0.8 : 0.6
                        text: model.comment
                        visible: model.comment.length > 0
                    }
                }
            }

            actions: [
                Kirigami.Action {
                    icon.name: "configure"
                    visible: model.configurable

                    onTriggered: kcm.pushConfigPage(model.name, "fcitx://config/addon/" + model.uniqueName)
                }
            ]
        }
    }
    footer: CheckBox {
        id: showEnable
        text: i18n("Show &Advanced options")

        onClicked: {
            showOptionWarning.visible = true;
        }
    }
}
