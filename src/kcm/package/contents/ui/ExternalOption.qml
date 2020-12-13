/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    property variant properties
    readonly property bool needsSave: false

    function load(rawValue) {}
    function save() {}
    function defaults() {}

    icon.name: "configure"
    onClicked: {
        kcm.launchExternal(properties.External)
    }
}
