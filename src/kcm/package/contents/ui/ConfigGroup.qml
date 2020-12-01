/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
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
                if (loader.item.hasOwnProperty("rawValue")) {
                    Utils.setRawValue(rawValue, loader.option.name,
                                      loader.item.rawValue)
                }
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
            Kirigami.FormData.labelAlignment: Qt.AlignTop

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
