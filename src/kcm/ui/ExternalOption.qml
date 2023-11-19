/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls

Button {
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    readonly property bool needsSave: false
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
