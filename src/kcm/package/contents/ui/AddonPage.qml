/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2 as KCM

KCM.ScrollViewKCM {
    property bool needsSave: false

    title: i18n("Addons")

    view: ListView {
        model: kcm.addonModel
        section {
            property: "categoryName"
            delegate: Kirigami.ListSectionHeader {
                label: section
            }
        }
        delegate: Kirigami.SwipeListItem {
            id: listItem
            actions: [
                Kirigami.Action {
                    icon.name: "configure"
                    onTriggered: kcm.pushConfigPage(
                                     model.name,
                                     "fcitx://config/addon/" + model.uniqueName)
                    visible: model.configurable
                }
            ]
            RowLayout {
                CheckBox {
                    id: itemChecked
                    Layout.leftMargin: Kirigami.Units.gridUnit
                    Layout.alignment: Qt.AlignVCenter
                    checked: model.enabled
                    onClicked: {
                        model.enabled = !model.enabled
                        needsSave = true
                    }
                }

                ColumnLayout {
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        text: model.name
                        level: 3
                        elide: Text.ElideRight
                    }

                    Label {
                        text: model.comment
                        opacity: listItem.hovered ? 0.8 : 0.6
                        visible: model.comment.length > 0
                    }
                }
            }
        }
    }

    header: RowLayout {
        TextField {
            Layout.fillWidth: true
            id: search
            placeholderText: i18n("Search...")
        }
    }

    Binding {
        target: kcm.addonModel
        property: "filterText"
        value: search.text
    }

    function load() {
        kcm.loadAddon()
        needsSave = false
    }
    function save() {
        kcm.saveAddon()
        needsSave = false
    }

    function defaults() {}

    function showWarning() {
        dialog.open()
    }

    SaveWarningDialog {
        id: dialog
    }
}
