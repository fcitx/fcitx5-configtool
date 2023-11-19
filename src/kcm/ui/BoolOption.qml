/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls

CheckBox {
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool needsSave: value !== checked
    property bool value: rawValue === "True"

    function load(rawValue) {
        checked = rawValue === "True";
    }
    function save() {
        rawValue = checked ? "True" : "False";
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }
}
