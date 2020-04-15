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

RowLayout {
    id: keyList

    property variant properties
    property variant rawValue
    property bool needsSave: false

    function load(rawValue) {
        needsSave = (rawValue !== keyList.rawValue)
        if (listModel.count !== 0) {
            listModel.remove(0, listModel.count)
        }
        var i = 0
        while (true) {
            if (!rawValue.hasOwnProperty(i.toString())) {
                break
            }
            var value = rawValue[i.toString()]
            listModel.append({
                                 "key": value
                             })
            i++
        }
    }

    function save() {
        var newRawValue = {}
        var j = 0
        for (var i = 0; i < listModel.count; i++) {
            console.log(listModel.get(i).key)
            if (listModel.get(i).key !== "") {
                newRawValue[j.toString()] = listModel.get(i).key
                j++
            }
        }
        rawValue = newRawValue
        needsSave = false
    }

    Component.onCompleted: {
        load(rawValue)
    }

    ColumnLayout {
        Repeater {
            model: listModel
            delegate: RowLayout {
                Layout.fillWidth: true
                KeyOption {
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
                    Layout.fillWidth: true
                    properties: keyList.properties.hasOwnProperty(
                                    "ListConstrain") ? keyList.properties.ListConstrain : {}
                    rawValue: model.key

                    onKeyStringChanged: {
                        model.key = keyString;
                    }
                }
                ToolButton {
                    icon.name: "list-remove"
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
        Layout.alignment: Qt.AlignVCenter
        icon.name: "list-add"
        onClicked: {
            keyList.needsSave = true;
            listModel.append({"key": ""});
        }
    }

    ListModel {
        id: listModel
    }
}
