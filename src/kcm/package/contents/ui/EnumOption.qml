/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14
import org.kde.kirigami 2.10 as Kirigami

ComboBox {
    // properties {{{
    property variant properties
    property variant rawValue
    property int value: computeValue(rawValue)
    property bool needsSave: value != currentIndex
    // }}}

    // functions {{{
    function computeValue(rawValue) {
        for (var i = 0; i < listModel.count; i++) {
            if (listModel.get(i).value == rawValue) {
                return i
            }
        }
        return 0
    }

    function load(rawValue) {
        currentIndex = computeValue(rawValue)
    }

    function save() {
        rawValue = properties["Enum"][currentIndex.toString()]
    }
    /// }}}

    implicitWidth: Kirigami.Units.gridUnit * 8
    textRole: "text"
    model: ListModel {
        id: listModel
    }
    Component.onCompleted: {
        var i = 0
        while (true) {
            var value = properties["Enum"][i.toString()]
            if (!value) {
                break
            }
            var text = ""
            if (properties.hasOwnProperty("EnumI18n")) {
                if (properties["EnumI18n"].hasOwnProperty(i.toString())) {
                    text = properties["EnumI18n"][i.toString()]
                }
            }
            if (text == "") {
                text = value
            }
            listModel.append({
                                 "text": text,
                                 "value": value
                             })
            i++
        }
        load(rawValue)
    }
}
