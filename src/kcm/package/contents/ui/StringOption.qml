/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

TextField {
    property bool needsSave: text !== rawValue
    property string rawValue

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
