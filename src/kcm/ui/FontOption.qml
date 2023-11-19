/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Button {
    id: button
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool needsSave: text !== rawValue
    property font value

    icon.name: "document-edit"
    text: kcm ? kcm.fontToString(value) : ""

    function load(rawValue) {
        value = kcm.parseFont(rawValue);
    }
    function save() {
        rawValue = text;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }
    onClicked: {
        fontDialog.selectedFont = value;
        fontDialog.open();
    }

    FontDialog {
        id: fontDialog
        title: i18nc("@title:window", "Select Font")
        parentWindow: button.Window.window

        onAccepted: {
            value = fontDialog.selectedFont;
        }
    }
}
