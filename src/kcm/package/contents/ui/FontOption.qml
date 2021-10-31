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

Button {
    property bool needsSave: text !== rawValue
    property variant rawValue

    icon.name: "document-edit"
    text: kcm.fontToString(fontDialog.font)

    function load(rawValue) {
        fontDialog.font = kcm.parseFont(rawValue);
    }
    function save() {
        rawValue = text;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }
    onClicked: fontDialog.open()

    QtDialogs.FontDialog {
        id: fontDialog
        title: i18nc("@title:window", "Select Font")
    }
}
