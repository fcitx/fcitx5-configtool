/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.10 as Kirigami

RowLayout {
    id: keyList
    property alias hovered: addButton.hovered
    property bool needsSave
    property variant properties
    property variant rawValue

    function load(rawValue) {
        var diff = false;
        if (listModel.count !== 0) {
            listModel.remove(0, listModel.count);
        }
        var i = 0;
        while (true) {
            if (!rawValue.hasOwnProperty(i.toString())) {
                break;
            }
            var value = rawValue[i.toString()];
            listModel.append({
                    "key": value
                });
            if (!keyList.rawValue.hasOwnProperty(i.toString()) || rawValue[i.toString()] !== keyList.rawValue[i.toString()]) {
                diff = true;
            }
            i++;
        }
        if (keyList.rawValue.hasOwnProperty(i.toString())) {
            diff = true;
        }
        needsSave = diff;
    }
    function save() {
        var newRawValue = {};
        var j = 0;
        for (var i = 0; i < listModel.count; i++) {
            if (listModel.get(i).key !== "") {
                newRawValue[j.toString()] = listModel.get(i).key;
                j++;
            }
        }
        rawValue = newRawValue;
        needsSave = false;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }

    ColumnLayout {
        Repeater {
            model: listModel

            delegate: RowLayout {
                KeyOption {
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 9
                    properties: keyList.properties.hasOwnProperty("ListConstrain") ? keyList.properties.ListConstrain : {}
                    rawValue: model.key

                    onKeyStringChanged: {
                        model.key = keyString;
                        keyList.needsSave = true;
                    }
                }
                ToolButton {
                    icon.name: "edit-delete-symbolic"

                    onClicked: {
                        // Need to happen before real remove.
                        keyList.needsSave = true;
                        listModel.remove(model.index);
                    }
                }
            }
        }
    }
    ToolButton {
        id: addButton
        Layout.alignment: Qt.AlignTop
        icon.name: "list-add-symbolic"

        onClicked: {
            keyList.needsSave = true;
            listModel.append({
                    "key": ""
                });
        }
    }
    ListModel {
        id: listModel
    }
}
