/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14

CheckBox {
    // properties {{{
    property variant rawValue
    property bool value: rawValue === "True"
    property bool needsSave: value !== checked
    // }}}

    // functions {{{
    function load(rawValue) {
        checked = rawValue === "True"
    }

    function save() {
        rawValue = checked ? "True" : "False"
    }
    // }}}

    Component.onCompleted: {
        load(rawValue)
    }
}
