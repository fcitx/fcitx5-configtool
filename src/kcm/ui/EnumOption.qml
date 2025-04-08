/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import "utils.js" as Utils

Row {
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool needsSave: value != comboBox.currentIndex
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
            if (!Utils.hasProperty(properties, "Enum")) {
                break;
            }
            if (!Utils.hasProperty(properties["Enum"], i.toString())) {
                break;
            }
            var value = properties["Enum"][i.toString()];
            var text = "";
            if (Utils.hasProperty(properties, "EnumI18n")) {
                if (Utils.hasProperty(properties["EnumI18n"], i.toString())) {
                    text = properties["EnumI18n"][i.toString()];
                }
            }
            if (text == "") {
                text = value;
            }
            var subconfigpath = "";
            if (Utils.hasProperty(properties, "SubConfigPath")) {
                if (Utils.hasProperty(properties["SubConfigPath"], i.toString())) {
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
        save();
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
