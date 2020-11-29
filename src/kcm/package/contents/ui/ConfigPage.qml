/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Controls 2.14
import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.1 as KCM

Kirigami.ScrollablePage {
    id: configPage

    property alias typeMap: configGroup.typeMap
    property alias typeName: configGroup.typeName
    property alias rawValue: configGroup.rawValue
    property alias needsSave: configGroup.needsSave
    property string uri

    function load() {
        configGroup.load()
    }
    function defaults() {
        configGroup.defaults()
    }
    function save() {
        configGroup.save()
        kcm.saveConfig(uri, rawValue)
    }

    function showWarning() {
        dialog.open()
    }

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
            onTriggered: configGroup.visible = true;
        }
    }
    Component.onCompleted: positionTimer.start()
}
