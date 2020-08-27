/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2 as KCM

KCM.ScrollViewKCM {
    // object properties {{{
    title: i18n("Add Input Method")

    view: ListView {
        id: availIMView
        model: kcm.imConfig.availIMModel
        section {
            property: "language"
            delegate: Kirigami.ListSectionHeader {
                label: section
            }
        }
        delegate: Kirigami.BasicListItem {
            label: model.name
            onClicked: {
                availIMView.currentIndex = index
            }
        }
    }

    header: RowLayout {
        TextField {
            id: search
            Layout.fillWidth: true
            placeholderText: i18n("Search...")
        }
    }

    footer: RowLayout {
        CheckBox {
            text: i18n("Only &Show Current Language")
            checked: true
            visible: search.text.length === 0
            onClicked: {
                kcm.imConfig.availIMModel.showOnlyCurrentLanguage = checked
            }
        }
        Item {
            Layout.fillWidth: true
        }
        Button {
            text: i18n("Add")
            icon.name: "list-add-symbolic"
            onClicked: {
                if (availIMView.currentIndex === -1) {
                    return
                }
                kcm.imConfig.addIM(availIMView.currentIndex)
                if (kcm.imConfig.currentIMModel.count === 1) {
                    kcm.mainUi.checkInputMethod();
                }
                kcm.pop()
            }
        }
    }

    Component.onCompleted: {
        search.forceActiveFocus();
    }

    // }}}

    Binding {
        target: kcm.imConfig.availIMModel
        property: "filterText"
        value: search.text
    }
}
