/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.10 as Kirigami

Row {
    property bool needsSave: value != comboBox.currentIndex
    property variant properties
    property variant rawValue
    property int value: computeValue(rawValue)

    function computeValue(rawValue) {
        for (var i = 0; i < listModel.count; i++) {
            if (listModel.get(i).value == rawValue) {
                return i;
            }
        }
        return 0;
    }
    function load(rawValue) {
        comboBox.currentIndex = computeValue(rawValue);
    }
    function save() {
        rawValue = properties["Enum"][comboBox.currentIndex.toString()];
    }

    Component.onCompleted: {
        var i = 0;
        while (true) {
            var value = properties["Enum"][i.toString()];
            if (!value) {
                break;
            }
            var text = "";
            if (properties.hasOwnProperty("EnumI18n")) {
                if (properties["EnumI18n"].hasOwnProperty(i.toString())) {
                    text = properties["EnumI18n"][i.toString()];
                }
            }
            if (text == "") {
                text = value;
            }
            var subconfigpath = "";
            if (properties.hasOwnProperty("SubConfigPath")) {
                if (properties["SubConfigPath"].hasOwnProperty(i.toString())) {
                    subconfigpath = properties["SubConfigPath"][i.toString()];
                }
            }
            listModel.append({
                    "text": text,
                    "value": value,
                    "subconfigpath": subconfigpath
                });
            i++;
        }
        load(rawValue);
    }

    ComboBox {
        id: comboBox
        implicitWidth: Kirigami.Units.gridUnit * 14
        textRole: "text"

        model: ListModel {
            id: listModel
        }
    }
    ToolButton {
        id: configureButton
        icon.name: "configure"
        visible: listModel.get(comboBox.currentIndex).subconfigpath !== ""

        onClicked: {
            kcm.pushConfigPage(listModel.get(comboBox.currentIndex).text, listModel.get(comboBox.currentIndex).subconfigpath);
        }
    }
}
