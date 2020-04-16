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
import QtQuick.Controls 2.14
import org.kde.kirigami 2.10 as Kirigami

ComboBox {
    // properties {{{
    property variant properties
    property variant rawValue
    property int value: computeValue(rawValue)
    property bool needSave: value != currentIndex
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
