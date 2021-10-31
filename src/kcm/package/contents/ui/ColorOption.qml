/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1 as QtDialogs
import org.kde.kirigami 2.10 as Kirigami

Row {
    property bool needsSave: button.text != rawValue
    property variant rawValue

    function load(rawValue) {
        colorDialog.color = kcm.parseColor(rawValue);
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
        text: kcm.colorToString(colorDialog.color)

        onClicked: colorDialog.open()

        QtDialogs.ColorDialog {
            id: colorDialog
            modality: Qt.ApplicationModal
            showAlphaChannel: true
            title: i18nc("@title:window", "Select Color")
        }
    }
    Rectangle {
        color: colorDialog.color
        height: button.height
        width: height
    }
}
