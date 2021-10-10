/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    property string description
    readonly property bool needsSave: false
    property variant properties
    property bool subConfig: false

    icon.name: "configure"

    function defaults() {
    }
    function load(rawValue) {
    }
    function save() {
    }

    Component.onCompleted: {
        if (properties.hasOwnProperty("LaunchSubConfig") && properties["LaunchSubConfig"] == "True") {
            subConfig = true;
        }
    }
    onClicked: {
        if (subConfig) {
            kcm.pushConfigPage(description, properties.External);
        } else {
            kcm.launchExternal(properties.External);
        }
    }
}
