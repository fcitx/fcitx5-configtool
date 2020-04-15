//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
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
