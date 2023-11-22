/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "utils.js" as Utils

Kirigami.FormLayout {
    id: configGroup
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool needsSave
    property variant typeMap

    function defaults() {
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i);
            if (loader.status == Loader.Ready) {
                loader.item.load(loader.option.defaultValue);
            }
        }
    }
    function load() {
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i);
            if (loader.status == Loader.Ready) {
                loader.item.load(loader.item.rawValue);
            }
        }
        needsSave = false;
    }
    function save() {
        var rawValue = {};
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i);
            if (loader.status == Loader.Ready) {
                loader.item.save();
                if (loader.item.hasOwnProperty("rawValue")) {
                    Utils.setRawValue(rawValue, loader.option.name, loader.item.rawValue);
                }
            }
        }
        configGroup.rawValue = rawValue;
        configGroup.needsSave = false;
    }
    function setRawValue(rawValue) {
        for (var i = 0; i < repeater.count; i++) {
            var loader = repeater.itemAt(i);
            if (loader.status == Loader.Ready) {
                loader.item.load(Utils.getRawValue(rawValue, loader.option.name));
            }
        }
    }

    Repeater {
        id: repeater
        model: typeMap[typeName]

        OptionLoader {
            id: loader
            Layout.fillWidth: option.type !== "Boolean"
            Kirigami.FormData.isSection: modelData.isSection
            Kirigami.FormData.label: modelData.isSection ? modelData.description : i18n("%1:", modelData.description)
            Kirigami.FormData.labelAlignment: !modelData.isSection && modelData.type.startsWith("List|") ? (height > Kirigami.Units.gridUnit * 2 ? Qt.AlignTop : 0) : 0
            option: modelData
            rawValue: modelData.isSection ? null : Utils.getRawValue(configGroup.rawValue, modelData.name)

            Connections {
                enabled: loader.status == Loader.Ready
                target: loader.status == Loader.Ready ? loader.item : null

                function onNeedsSaveChanged() {
                    if (target.needsSave) {
                        configGroup.needsSave = true;
                    }
                }
            }
        }
    }
}
