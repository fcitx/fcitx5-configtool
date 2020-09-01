/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14

Button {
    property variant properties
    readonly property bool needsSave: false

    function load(rawValue) {}
    function save() {}
    function defaults() {}

    icon.name: "configure"
    ToolTip.visible: option.properties.hasOwnProperty("Tooltip") ? hovered : hovered
    ToolTip.delay: Kirigami.Units.toolTipDelay
    ToolTip.text: option.properties.hasOwnProperty("Tooltip") ? option.properties["Tooltip"] : ""
    onClicked: {
        kcm.launchExternal(properties.External)
    }
}
