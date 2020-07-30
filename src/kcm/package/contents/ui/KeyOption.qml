/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14
import "utils.js" as Utils

Button {
    id: root

    // properties {{{
    property variant properties
    property variant rawValue
    property bool needsSave: keyString !== rawValue
    property bool grab: false
    property bool allowModifierLess: false
    property bool allowModifierOnly: false
    property string keyString: ""
    property string currentKeyString: ""
    // }}}

    function load(rawValue) {
        keyString = rawValue
    }

    function save() {
        rawValue = keyString
    }

    function isOkWhenModifierless(event) {
        if (event.text.length === 1) {
            return false
        }

        if (event.key == Qt.Key_Return || event.key == Qt.Key_Space
                || event.key == Qt.Key_Tab || event.key == Qt.Key_Backtab
                || event.key == Qt.Key_Backspace
                || event.key == Qt.Key_Delete) {
            return false
        }
        return true
    }

    function prettyKeyString(keyString) {
        if (keyString === "") {
            if (grab) {
                return i18n("...")
            } else {
                return i18n("Empty")
            }
        }
        return kcm.localizedKeyString(keyString)
    }

    flat: false
    down: grab
    focus: grab
    text: prettyKeyString(grab ? currentKeyString : keyString)
    onClicked: {
        if (down) {
            kcm.ungrabKeyboard(root)
            grab = false
        } else {
            grab = true
            currentKeyString = ""
            kcm.grabKeyboard(root)
        }
    }
    onPressAndHold: {
        contextMenu.popup()
    }
    Keys.onPressed: {
        event.accepted = true
        if (!grab) {
            return
        }
        var done = true
        currentKeyString = kcm.eventToString(event.key, event.modifiers,
                                             event.nativeScanCode, event.text,
                                             keyCodeAction.checked)
        if (text === "") {
            done = false
        }
        var modifiers = event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier
                                           | Qt.AltModifier | Qt.MetaModifier)
        if ((modifiers & ~Qt.ShiftModifier) == 0) {
            if (!isOkWhenModifierless(event) && !allowModifierLess) {
                done = false
            }
        }
        if ((event.key == Qt.Key_Shift || event.key == Qt.Key_Control
             || event.key == Qt.Key_Meta || event.key == Qt.Key_Super_L
             || event.key == Qt.Key_Super_R || event.key == Qt.Key_Alt)) {
            done = false
        }
        if (done) {
            keyString = currentKeyString;
            onClicked()
        }
    }
    Keys.onReleased: {
        event.accepted = true
        if (!grab) {
            return
        }
        var done = false
        if (allowModifierOnly
                && (event.key == Qt.Key_Shift || event.key == Qt.Key_Control
                    || event.key == Qt.Key_Meta || event.key == Qt.Key_Super_L
                    || event.key == Qt.Key_Super_R
                    || event.key == Qt.Key_Alt)) {
            done = true
        }
        var keyStr = kcm.eventToString(event.key, event.modifiers,
                                       event.nativeScanCode, event.text,
                                       keyCodeAction.checked)
        if (keyStr === "") {
            done = false
        }

        if (done) {
            keyString = keyStr
            onClicked()
        } else {
            if (event.modifiers == 0) {
                currentKeyString = ""
            } else {
                currentKeyString = keyStr
            }
        }
    }
    Component.onCompleted: {
        if (properties.hasOwnProperty("AllowModifierLess")) {
            allowModifierLess = properties.AllowModifierLess == "True"
        }
        if (properties.hasOwnProperty("AllowModifierOnly")) {
            allowModifierOnly = properties.AllowModifierOnly == "True"
        }
        load(rawValue)
    }

    Menu {
        id: contextMenu

        Action {
            id: keyCodeAction
            text: i18n("Key code mode")
            checkable: true
        }
    }
}
