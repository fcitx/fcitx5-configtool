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

ColumnLayout {
    id: listOption
    property string typeName
    property string description
    property variant defaultValue
    property variant properties
    property variant rawValue
    property alias hovered: addButton.hovered
    property bool needsSave: false
    readonly property string subTypeName: typeName.substr(5)

    function getOption() {
        var option = {};
        option.isSection = false;
        option.type = subTypeName;
        option.properties = properties;
        option.defaultValue = "";
        option.name = [];
        return option;
    }
    function load(rawValue) {
        needsSave = (rawValue !== listOption.rawValue);
        if (listModel.count !== 0) {
            listModel.remove(0, listModel.count);
        }
        var i = 0;
        while (true) {
            if (!rawValue.hasOwnProperty(i.toString())) {
                break;
            }
            var value = rawValue[i.toString()];
            listModel.append({
                    "value": value
                });
            i++;
        }
    }
    function prettify(value, subType) {
        if (subType == "Integer") {
            return value;
        } else if (subType == "String") {
            return value;
        } else if (subType == "Boolean") {
            return value == "True" ? i18n("Yes") : i18n("No");
        } else if (subType == "Key") {
            return kcm ? kcm.localizedKeyString(value) : "";
        } else if (subType == "Enum") {
            var i = 0;
            var enumMap = {};
            while (true) {
                if (!properties.Enum.hasOwnProperty(i.toString())) {
                    break;
                }
                var enumString = properties.Enum[i.toString()];
                var text = enumString;
                if (properties.hasOwnProperty("EnumI18n") && properties.EnumI18n.hasOwnProperty(i.toString())) {
                    text = properties.EnumI18n[i.toString()];
                }
                enumMap[enumString] = text;
                i++;
            }
            return enumMap[value];
        } else if (subType.startsWith("List|")) {
            var i = 0;
            subSubType = subType.substr(5);
            var strs = [];
            while (true) {
                if (!value.hasOwnProperty(i.toString())) {
                    break;
                }
                var subValue = prettify(value[i.toString()]);
                strs.push(subValue);
                i++;
            }
            return i18n("[%1]", strs.join(" "));
        } else if (configPage.typeMap.hasOwnProperty(subType)) {
            for (var i = 0; i < configPage.typeMap[subTypeName].length; ++i) {
                var option = configPage.typeMap[subTypeName][i];
                if (option.name.length === 1 && option.name[0] === properties.ListDisplayOption) {
                    return prettify(value[properties.ListDisplayOption], option.type);
                }
            }
        }
        return "";
    }
    function save() {
        var newRawValue = {};
        var j = 0;
        for (var i = 0; i < listModel.count; i++) {
            if (listModel.get(i).value !== "") {
                newRawValue[j.toString()] = listModel.get(i).value;
                j++;
            }
        }
        rawValue = newRawValue;
    }

    Component.onCompleted: {
        load(rawValue);
        save();
    }

    Component {
        id: delegateComponent

        ItemDelegate {
            width: ListView.view ? ListView.view.width : implicitWidth
            height: listItem.implicitHeight
            Kirigami.SwipeListItem {
                id: listItem
                width: parent.width

                contentItem: RowLayout {
                    Kirigami.ListItemDragHandle {
                        listItem: listItem
                        listView: optionView

                        onMoveRequested: (oldIndex, newIndex) => {
                            needsSave = true;
                            listModel.move(oldIndex, newIndex, 1);
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        color: listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
                        height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
                        text: model !== null ? prettify(model.value, subTypeName) : ""
                        elide: Text.ElideRight
                    }
                }

                actions: [
                    Kirigami.Action {
                        icon.name: "document-edit"
                        text: i18n("edit")

                        onTriggered: {
                            sheet.edit(model.index);
                        }
                    },
                    Kirigami.Action {
                        icon.name: "list-remove-symbolic"
                        text: i18n("Remove")

                        onTriggered: {
                            needsSave = true;
                            listModel.remove(model.index);
                        }
                    }
                ]
            }
        }
    }
    ListModel {
        id: listModel
    }
    ScrollView {
        Layout.fillWidth: true
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        implicitHeight: optionView.contentHeight
        implicitWidth: Kirigami.Units.gridUnit * 10

        ListView {
            id: optionView
            model: listModel

            delegate: delegateComponent
            reuseItems: true
        }
    }
    RowLayout {
        ToolButton {
            id: addButton
            icon.name: "list-add-symbolic"
            text: i18n("Add")

            onClicked: {
                sheet.edit(listModel.count);
            }
        }
    }
    Kirigami.OverlaySheet {
        id: sheet
        property int editIndex: -1
        readonly property bool isSubConfig: configPage.typeMap.hasOwnProperty(subTypeName)

        parent: configPage

        function edit(index) {
            editIndex = index;
            if (editIndex < listModel.count) {
                if (isSubConfig) {
                    item().setRawValue(listModel.get(sheet.editIndex).value);
                } else {
                    item().load(listModel.get(sheet.editIndex).value);
                }
            } else {
                if (isSubConfig) {
                    item().defaults();
                } else {
                    item().load("");
                }
            }
            sheet.open();
        }
        function item() {
            return isSubConfig ? optionEditor.item : optionEditor.item.item;
        }

        Loader {
            id: optionEditor
            sourceComponent: sheet.isSubConfig ? group : option
        }

        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel

            onAccepted: {
                sheet.item().save();
                // Add a new one.
                if (sheet.editIndex == listModel.count) {
                    listModel.append({
                            "value": sheet.item().rawValue
                        });
                } else {
                    listModel.get(sheet.editIndex).value = sheet.item().rawValue;
                }
                sheet.close();
                needsSave = true;
            }
            onRejected: sheet.close()
        }
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
            rawValue: null
            typeMap: configPage.typeMap
            typeName: subTypeName
        }
    }
}
