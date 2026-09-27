/*
 * SPDX-FileCopyrightText: 2025~2025 The fcitx5-configtool authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import "utils.js" as Utils

RowLayout {
    id: optionalOption
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property alias hovered: toggle.hovered
    property bool hasValue: false
    property bool needsSave: toggle.checked !== hasValue || (toggle.checked && item() && item().needsSave)
    readonly property string subTypeName: typeName.substr(9)
    readonly property bool valid: !toggle.checked || (item() && (!Utils.hasProperty(item(), "valid") || item().valid))

    function onNeedsSaveChanged() {
        console.log("needsSave changed: " + needsSave);
    }

    function getOption() {
        var option = {};
        option.isSection = false;
        option.type = subTypeName;
        option.properties = properties;
        option.defaultValue = "";
        option.name = [];
        return option;
    }

    function load(rawValue) {
        hasValue = Utils.hasProperty(rawValue, "Value");
        toggle.checked = hasValue;
        if (hasValue) {
            item().load(rawValue.Value);
        }
    }

    function save() {
        var newRawValue = {};
        if (toggle.checked) {
            var editor = item();
            if (!editor || (Utils.hasProperty(editor, "valid") && !editor.valid) || editor.save() === false) {
                return false;
            }
            newRawValue["Value"] = editor.rawValue;
        }
        rawValue = newRawValue;
        return true;
    }

    function item() {
        return optionEditor.item ? optionEditor.item.item : null;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }

    Switch {
        id: toggle
    }

    Loader {
        id: optionEditor
        Layout.fillWidth: true
        enabled: toggle.checked
        visible: toggle.checked
        sourceComponent: optionComponent
    }

    Component {
        id: optionComponent
        OptionLoader {
            option: getOption()
            rawValue: ""
        }
    }
}
