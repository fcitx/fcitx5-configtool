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
    property string description
    readonly property bool needsSave: false
    property bool subConfig: false

    function load(rawValue) {}
    function save() {}
    function defaults() {}

    icon.name: "configure"
    onClicked: {
        if (subConfig) {
            kcm.pushConfigPage(description,
                               properties.External)
        } else {
            kcm.launchExternal(properties.External)
        }
    }

    Component.onCompleted: {
        if (properties.hasOwnProperty("LaunchSubConfig") && properties["LaunchSubConfig"] == "True") {
            subConfig = true
        }
    }
}
