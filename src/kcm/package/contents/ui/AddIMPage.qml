//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
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
            icon.name: "list-add"
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
    // }}}

    Binding {
        target: kcm.imConfig.availIMModel
        property: "filterText"
        value: search.text
    }
}
