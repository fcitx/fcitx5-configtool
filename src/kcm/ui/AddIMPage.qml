/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    title: i18n("Add Input Method")

    Component.onCompleted: {
        search.forceActiveFocus();
    }

    Binding {
        property: "filterText"
        target: kcm ? kcm.imConfig.availIMModel : null
        value: search.text
    }

    footer: RowLayout {
        CheckBox {
            checked: true
            text: i18n("Only &Show Current Language")
            visible: search.text.length === 0

            onClicked: {
                kcm.imConfig.availIMModel.showOnlyCurrentLanguage = checked;
            }
        }
        Item {
            Layout.fillWidth: true
        }
        Button {
            icon.name: "list-add-symbolic"
            text: i18n("Add")

            onClicked: {
                if (availIMView.currentIndex === -1) {
                    return;
                }
                kcm.imConfig.addIM(availIMView.currentIndex);
                if (kcm.imConfig.currentIMModel.count === 1) {
                    kcm.mainUi.checkInputMethod();
                }
                kcm.pop();
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
    view: ListView {
        id: availIMView
        model: kcm ? kcm.imConfig.availIMModel : null

        section {
            property: "language"

            delegate: Kirigami.ListSectionHeader {
                label: section
            }
        }

        delegate: ItemDelegate {
            text: model.name
            Layout.fillWidth: true

            onClicked: {
                availIMView.currentIndex = index;
            }
        }
    }
}
