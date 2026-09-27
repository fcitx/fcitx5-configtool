/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

Kirigami.ScrollablePage {
    id: configPage
    property alias needsSave: configGroup.needsSave
    property alias rawValue: configGroup.rawValue
    property alias typeMap: configGroup.typeMap
    property alias typeName: configGroup.typeName
    property alias valid: configGroup.valid
    property string uri

    function defaults() {
        configGroup.defaults();
    }
    function load() {
        configGroup.load();
    }
    function save() {
        if (!configGroup.save()) {
            return false;
        }
        kcm.saveConfig(uri, rawValue);
        return true;
    }
    function showWarning() {
        dialog.open();
    }

    Component.onCompleted: positionTimer.start()

    ConfigGroup {
        id: configGroup
        visible: false
        width: parent.width

        SaveWarningDialog {
            id: dialog
            parent: configPage
        }

        // Hack for force relayout the scrollview
        Timer {
            id: positionTimer
            interval: 0
            repeat: false

            onTriggered: configGroup.visible = true
        }
    }
}
