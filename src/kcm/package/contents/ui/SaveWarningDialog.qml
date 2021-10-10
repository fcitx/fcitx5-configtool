/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
    id: notSavedDialog
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    focus: true
    modal: true
    standardButtons: Dialog.Ok
    title: i18n("Some config is not saved")
    x: (parent.width - width) / 2
    y: parent.height / 2 - height

    Label {
        text: i18n("Current page is not yet saved. Please save the config first.")
    }

    Overlay.modal: Rectangle {
        color: "#99000000"
    }
}
