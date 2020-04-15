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
import org.kde.kirigami 2.10 as Kirigami

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

        SaveWarningDialog {
            id: dialog

            parent: configPage
        }
    }
}
