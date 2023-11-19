/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs
import org.kde.kirigami as Kirigami

Row {
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool needsSave: button.text != rawValue
    property color value

    function load(rawValue) {
        value = kcm.parseColor(rawValue);
    }
    function save() {
        rawValue = button.text;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }

    Button {
        id: button
        icon.name: "document-edit"
        implicitWidth: Kirigami.Units.gridUnit * 10
        text: kcm ? kcm.colorToString(value) : ""
        

        onClicked: {
            colorDialog.selectedColor = value;
            colorDialog.open();
        }

        QtDialogs.ColorDialog {
            id: colorDialog
            modality: Qt.ApplicationModal
            options: QtDialogs.ColorDialog.ShowAlphaChannel
            parentWindow: button.Window.window

            title: i18nc("@title:window", "Select Color")

            onAccepted: {
                value = colorDialog.selectedColor;
            }
        }
    }
    Rectangle {
        color: value
        height: button.height
        width: height
    }
}
