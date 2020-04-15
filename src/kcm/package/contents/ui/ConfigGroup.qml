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
import QtQuick.Layouts 1.14
import org.kde.kirigami 2.10 as Kirigami
import "utils.js" as Utils

Kirigami.FormLayout {
    id: configGroup

    property variant typeMap
    property string typeName
    property variant rawValue
    property bool needsSave

    function load() {
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i)
            if (loader.status == Loader.Ready) {
                loader.item.load(loader.item.rawValue)
            }
        }
        needsSave = false
    }

    function defaults() {
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i)
            if (loader.status == Loader.Ready) {
                loader.item.load(loader.option.defaultValue)
            }
        }
    }

    function save() {
        var rawValue = {}
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i)
            if (loader.status == Loader.Ready) {
                loader.item.save()
                Utils.setRawValue(rawValue, loader.option.name,
                                  loader.item.rawValue)
            }
        }
        configGroup.rawValue = rawValue
        configGroup.needsSave = false
    }

    Repeater {
        id: repeater

        model: typeMap[typeName]

        OptionLoader {
            id: loader

            option: modelData
            Kirigami.FormData.isSection: modelData.isSection
            Kirigami.FormData.label: modelData.description

            Connections {
                target: loader.status == Loader.Ready ? loader.item : null
                enabled: loader.status == Loader.Ready

                function onNeedsSaveChanged() {
                    if (target.needsSave) {
                        configGroup.needsSave = true
                    }
                }
            }
        }
    }
}
