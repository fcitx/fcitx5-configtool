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

Loader {
    id: loader
    property variant option
    property variant rawValue

    ToolTip.delay: Kirigami.Units.toolTipDelay
    ToolTip.text: option.properties && option.properties.hasOwnProperty("Tooltip") ? option.properties["Tooltip"] : ""
    ToolTip.visible: {
        if (option.properties && option.properties.hasOwnProperty("Tooltip") && loader.item) {
            if (loader.item.hasOwnProperty("hovered")) {
                return loader.item.hovered;
            }
        }
        return false;
    }

    function optionSource(data) {
        if (data.type == "Boolean") {
            return "BoolOption.qml";
        } else if (data.type == "Integer") {
            return "IntegerOption.qml";
        } else if (data.type == "Enum") {
            return "EnumOption.qml";
        } else if (data.type == "String") {
            if (data.properties.hasOwnProperty("Font") && data.properties.Font == "True") {
                return "FontOption.qml";
            } else if (data.properties.hasOwnProperty("IsEnum") && data.properties.IsEnum == "True") {
                return "EnumOption.qml";
            } else {
                return "StringOption.qml";
            }
        } else if (data.type == "List|Key") {
            return "KeyListOption.qml";
        } else if (data.type == "Key") {
            return "KeyOption.qml";
        } else if (data.type == "Color") {
            return "ColorOption.qml";
        } else if (data.type.startsWith("List|")) {
            return "ListOption.qml";
        } else if (data.type == "External") {
            return "ExternalOption.qml";
        } else {
            console.log(data.type + " Not supported");
            return "";
        }
    }

    Component.onCompleted: {
        if (!option.isSection) {
            var prop = {
                "typeName": option.type,
                "description": option.description ? option.description : "",
                "defaultValue": option.defaultValue,
                "properties": option.properties,
                "rawValue": rawValue
            };
            loader.setSource(optionSource(option), prop);
        }
    }
}
