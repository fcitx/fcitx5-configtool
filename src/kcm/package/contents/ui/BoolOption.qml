/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

CheckBox {
    property bool needsSave: value !== checked
    property variant rawValue
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
