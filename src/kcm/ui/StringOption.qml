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

TextField {
    id: root
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property string rawValue
    property bool needsSave: text !== rawValue
    readonly property bool isRegex: properties && ((Utils.hasProperty(properties, "IsRegex") && properties.IsRegex === "True") || (Utils.hasProperty(properties, "ListConstrain") && Utils.hasProperty(properties.ListConstrain, "IsRegex") && properties.ListConstrain.IsRegex === "True"))
    readonly property bool valid: !isRegex || (kcm && kcm.isValidRegex(text))

    function load(rawValue) {
        text = rawValue;
    }
    function save() {
        if (!valid) {
            return false;
        }
        rawValue = text;
        return true;
    }

    states: State {
        when: !root.valid
        PropertyChanges {
            target: root.Kirigami.Theme
            backgroundColor: Kirigami.Theme.negativeBackgroundColor
            textColor: Kirigami.Theme.highlightedTextColor
        }
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }
}
