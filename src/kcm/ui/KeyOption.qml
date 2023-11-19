/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import "utils.js" as Utils

Button {
    id: root
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property bool allowModifierLess: false
    property bool allowModifierOnly: false
    property string currentKeyString: ""
    property bool grab: false
    property string keyString: ""
    property bool needsSave: keyString !== rawValue

    down: grab
    flat: false
    focus: grab
    text: prettyKeyString(grab ? currentKeyString : keyString)

    function accept() {
        kcm.ungrabKeyboard(root);
        grab = false;
    }
    function isOkWhenModifierless(event) {
        var isSpecialKey = event.key == Qt.Key_Return || event.key == Qt.Key_Space || event.key == Qt.Key_Tab || event.key == Qt.Key_Backtab || event.key == Qt.Key_Backspace || event.key == Qt.Key_Delete;
        if (isSpecialKey) {
            if ((event.modifiers & Qt.ShiftModifier) != 0) {
                return true;
            }
            return false;
        } else if (event.text.length === 1) {
            return false;
        }
        return true;
    }
    function load(rawValue) {
        keyString = rawValue;
    }
    function prettyKeyString(keyString) {
        if (keyString === "") {
            if (grab) {
                return i18n("...");
            } else {
                return i18n("Empty");
            }
        }
        return kcm.localizedKeyString(keyString);
    }
    function save() {
        rawValue = keyString;
    }

    Component.onCompleted: {
        if (properties.hasOwnProperty("AllowModifierLess")) {
            allowModifierLess = properties.AllowModifierLess == "True";
        }
        if (properties.hasOwnProperty("AllowModifierOnly")) {
            allowModifierOnly = properties.AllowModifierOnly == "True";
        }
        load(rawValue);
        save();
    }
    Keys.onPressed: {
        event.accepted = true;
        if (!grab) {
            return;
        }
        var done = true;
        currentKeyString = kcm.eventToString(event.key, event.modifiers, event.nativeScanCode, event.text, keyCodeAction.checked);
        if (text === "") {
            done = false;
        }
        var modifiers = event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier);
        if ((modifiers & ~Qt.ShiftModifier) == 0) {
            if (!isOkWhenModifierless(event) && !allowModifierLess) {
                done = false;
            }
        }
        if ((event.key == Qt.Key_Shift || event.key == Qt.Key_Control || event.key == Qt.Key_Meta || event.key == Qt.Key_Super_L || event.key == Qt.Key_Super_R || event.key == Qt.Key_Hyper_L || event.key == Qt.Key_Hyper_R || event.key == Qt.Key_Alt)) {
            done = false;
        }
        if (done) {
            keyString = currentKeyString;
            accept();
        }
    }
    Keys.onReleased: {
        event.accepted = true;
        if (!grab) {
            return;
        }
        var done = false;
        if (allowModifierOnly && (event.key == Qt.Key_Shift || event.key == Qt.Key_Control || event.key == Qt.Key_Meta || event.key == Qt.Key_Super_L || event.key == Qt.Key_Super_R || event.key == Qt.Key_Hyper_L || event.key == Qt.Key_Hyper_R || event.key == Qt.Key_Alt)) {
            done = true;
        }
        var keyStr = kcm.eventToString(event.key, event.modifiers, event.nativeScanCode, event.text, keyCodeAction.checked);
        if (keyStr === "") {
            done = false;
        }
        if (done) {
            keyString = keyStr;
            accept();
        } else {
            if (event.modifiers == 0) {
                currentKeyString = "";
            } else {
                currentKeyString = keyStr;
            }
        }
    }
    Keys.onShortcutOverride: {
        event.accepted = true;
        Keys.onPressed(event);
    }
    onClicked: {
        if (down) {
            keyString = "";
            accept();
        } else {
            grab = true;
            currentKeyString = "";
            kcm.grabKeyboard(root);
        }
    }
    onPressAndHold: {
        contextMenu.popup();
    }

    Menu {
        id: contextMenu
        Action {
            id: keyCodeAction
            checkable: true
            text: i18n("Key code mode")
        }
    }
}
