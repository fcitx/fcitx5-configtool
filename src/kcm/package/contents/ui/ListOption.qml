/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.10 as Kirigami

ColumnLayout {
    id: listOption

    property variant properties
    property variant rawValue
    property string typeName
    property bool needsSave: false
    property alias hovered: addButton.hovered
    readonly property string subTypeName: typeName.substr(5)

    // functions {{{
    function getOption() {
        var option = {}
        option.isSection = false
        option.type = subTypeName
        option.properties = properties
        option.defaultValue = ""
        option.name = []
        return option
    }

    function prettify(value, subType) {
        if (subType == "Integer") {
            return value
        } else if (subType == "String") {
            return value
        } else if (subType == "Boolean") {
            return value == "True" ? i18n("Yes") : i18n("No")
        } else if (subType == "Key") {
            return kcm.localizedKeyString(value)
        } else if (subType == "Enum") {
            var i = 0
            var enumMap = {}
            while (true) {
                if (!properties.Enum.hasOwnProperty(i.toString())) {
                    break
                }

                var enumString = properties.Enum[i.toString()]
                var text = enumString

                if (properties.hasOwnProperty("EnumI18n")
                        && properties.EnumI18n.hasOwnProperty(i.toString())) {
                    text = properties.EnumI18n[i.toString()]
                }
                enumMap[enumString] = text
                i++
            }
            return enumMap[value]
        } else if (subType.startsWith("List|")) {
            var i = 0
            subSubType = subType.substr(5)
            var strs = []
            while (true) {
                if (!value.hasOwnProperty(i.toString())) {
                    break
                }
                var subValue = prettify(value[i.toString()])
                strs.push(subValue)
                i++
            }
            return i18n("[%1]", strs.join(" "))
        } else if (configPage.typeMap.hasOwnProperty(subType)) {
            for (var i = 0; i < configPage.typeMap[subTypeName].length; ++i) {
                var option = configPage.typeMap[subTypeName][i]
                if (option.name.length === 1 && option.name[0] === properties.ListDisplayOption) {
                    return prettify(value[properties.ListDisplayOption], option.type)
                }
            }
        }
        return ""
    }

    function load(rawValue) {
        needsSave = (rawValue !== listOption.rawValue)
        if (listModel.count !== 0) {
            listModel.remove(0, listModel.count)
        }
        var i = 0
        while (true) {
            if (!rawValue.hasOwnProperty(i.toString())) {
                break
            }
            var value = rawValue[i.toString()]
            listModel.append({
                                 "value": value
                             })
            i++
        }
    }

    function save() {
        var newRawValue = {}
        var j = 0
        for (var i = 0; i < listModel.count; i++) {
            if (listModel.get(i).value !== "") {
                newRawValue[j.toString()] = listModel.get(i).value
                j++
            }
        }
        rawValue = newRawValue
    }
    // }}}

    Component {
        id: delegateComponent
        Kirigami.SwipeListItem {
            id: listItem

            actions: [
                Kirigami.Action {
                    iconName: "document-edit"
                    text: i18n("edit")
                    onTriggered: {
                        sheet.edit(model.index);
                    }
                },
                Kirigami.Action {
                    iconName: "list-remove-symbolic"
                    text: i18n("Remove")
                    onTriggered: {
                        needsSave = true;
                        listModel.remove(model.index);
                    }
                }
            ]

            RowLayout {
                Kirigami.ListItemDragHandle {
                    listItem: listItem
                    listView: optionView
                    onMoveRequested: {
                        needsSave = true;
                        listModel.move(oldIndex, newIndex, 1);
                    }
                }
                Label {
                    Layout.fillWidth: true
                    height: Math.max(implicitHeight,
                                     Kirigami.Units.iconSizes.smallMedium)
                    text: model !== null ? prettify(model.value,
                                                    subTypeName) : ""
                    color: listItem.checked
                           || (listItem.pressed && !listItem.checked
                               && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
                }
            }
        }
    }

    ListModel {
        id: listModel
    }

    ScrollView {
        implicitWidth: Kirigami.Units.gridUnit * 10
        implicitHeight: optionView.contentHeight
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        ListView {
            id: optionView

            model: listModel
            delegate: Kirigami.DelegateRecycler {
                width: optionView.width
                sourceComponent: delegateComponent
            }
        }
    }

    RowLayout {
        ToolButton {
            id: addButton
            icon.name: "list-add-symbolic"
            text: i18n("Add")
            onClicked: {
                sheet.edit(listModel.count)
            }
        }
    }

    Kirigami.OverlaySheet {
        id: sheet

        property int editIndex: -1
        readonly property bool isSubConfig: configPage.typeMap.hasOwnProperty(subTypeName)

        function item() {
            return isSubConfig ? optionEditor.item : optionEditor.item.item
        }

        function edit(index) {
            editIndex = index
            if (editIndex < listModel.count) {
                if (isSubConfig) {
                    item().setRawValue(listModel.get(sheet.editIndex).value)
                } else {
                    item().load(listModel.get(sheet.editIndex).value)
                }
            } else {
                if (isSubConfig) {
                    item().defaults()
                } else {
                    item().load("")
                }
            }
            sheet.open()
        }

        parent: configPage

        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            onAccepted: {
                sheet.item().save();
                // Add a new one.
                if (sheet.editIndex == listModel.count) {
                    listModel.append({"value": sheet.item().rawValue});
                } else {
                    listModel.get(sheet.editIndex).value = sheet.item().rawValue;
                }
                sheet.close();
                needsSave = true;
            }
            onRejected: sheet.close()
        }

        Loader {
            id: optionEditor
            sourceComponent: sheet.isSubConfig ? group : option
        }
    }

    Component.onCompleted: {
        load(rawValue)
    }

    Component {
        id: option
        OptionLoader {
            option: getOption()
            rawValue: ""
        }
    }

    Component {
        id: group
        ConfigGroup {
            typeMap: configPage.typeMap
            typeName: subTypeName
            rawValue: null
        }
    }
}
