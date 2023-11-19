/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls

TextField {
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property string rawValue
    property bool needsSave: text !== rawValue

    function load(rawValue) {
        text = rawValue;
    }
    function save() {
        rawValue = text;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }
}
