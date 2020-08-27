/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.1 as QtDialogs

Button {
    property variant rawValue
    property bool needsSave: text != rawValue

    icon.name: "document-edit"
    text: kcm.fontToString(fontDialog.font)
    onClicked: fontDialog.open()

    ToolTip {
        visible: parent.hovered
        text: i18n("Select Font...")
    }

    QtDialogs.FontDialog {
        id: fontDialog
        title: i18nc("@title:window", "Select Font")
    }

    Component.onCompleted: {
        load(rawValue)
    }

    function load(rawValue) {
        fontDialog.font = kcm.parseFont(rawValue)
    }

    function save() {
        rawValue = text
    }
}
