/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import "utils.js" as Utils

SpinBox {
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool needsSave: value !== oldValue
    property int oldValue: parseInt(rawValue)

    from: validator.bottom
    to: validator.top

    function load(rawValue) {
        value = parseInt(rawValue);
    }
    function save() {
        rawValue = value.toString();
    }

    Component.onCompleted: {
        if (Utils.hasProperty(properties, "IntMin")) {
            validator.bottom = parseInt(properties.IntMin);
        }
        if (Utils.hasProperty(properties, "IntMax")) {
            validator.top = parseInt(properties.IntMax);
        }
        load(rawValue);
        save();
    }

    validator: IntValidator {
        id: validator
    }
}
