/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

SpinBox {
    property variant properties
    property variant rawValue
    property int oldValue: parseInt(rawValue)
    property bool needsSave: value !== oldValue

    function load(rawValue) {
        value = parseInt(rawValue)
    }

    function save() {
        rawValue = value.toString()
    }

    Component.onCompleted: {
        if (properties.hasOwnProperty("IntMin")) {
            validator.bottom = parseInt(properties.IntMin)
        }
        if (properties.hasOwnProperty("IntMax")) {
            validator.top = parseInt(properties.IntMax)
        }
        load(rawValue)
    }

    from: validator.bottom
    to: validator.top
    validator: IntValidator {
        id: validator
    }
}
