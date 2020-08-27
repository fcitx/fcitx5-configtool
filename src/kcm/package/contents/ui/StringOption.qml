/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14

TextField {
    // properties {{{
    property variant rawValue
    property bool needsSave: text !== rawValue
    // }}}

    // functions {{{
    function load(rawValue) {
        text = rawValue
    }

    function save() {
        rawValue = text
    }
    // }}}

    Component.onCompleted: {
        load(rawValue)
    }
}
