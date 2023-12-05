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
    id: root
    Kirigami.ColumnView.fillWidth: true
    implicitHeight: Kirigami.Units.gridUnit * 36
    implicitWidth: Kirigami.Units.gridUnit * 50

    function checkInputMethod() {
        var firstIM = imList.model.imAt(0);
        inputMethodNotMatchWarning.visible = false;
        if (firstIM.startsWith("keyboard-")) {
            layoutNotMatchWarning.visible = (firstIM.substr(9) != kcm.imConfig.defaultLayout);
        } else {
            layoutNotMatchWarning.visible = false;
        }
    }

    SelectLayoutSheet {
        id: selectLayoutSheet
        parent: root
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
                        listView: imList

                        onMoveRequested: (oldIndex, newIndex) => {
                            imList.model.move(oldIndex, newIndex);
                            checkInputMethod();
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        color: listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
                        height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
                        text: model !== null ? model.name : ""
                    }
                }

                actions: [
                    Kirigami.Action {
                        icon.name: "configure"
                        text: i18n("Configure")
                        visible: model !== null ? model.configurable : false

                        onTriggered: kcm.pushConfigPage(model.name, "fcitx://config/inputmethod/" + model.uniqueName)
                    },
                    Kirigami.Action {
                        icon.name: "input-keyboard"
                        text: i18n("Select Layout")
                        visible: model !== null ? !model.uniqueName.startsWith("keyboard-") : false

                        onTriggered: selectLayoutSheet.selectLayout(i18n("Select layout for %1", model.name), model.uniqueName, (model.layout !== "" ? model.layout : kcm.imConfig.defaultLayout))
                    },
                    Kirigami.Action {
                        icon.name: "list-remove-symbolic"
                        text: i18n("Remove")

                        onTriggered: {
                            imList.model.remove(model.index);
                            checkInputMethod();
                        }
                    }
                ]
            }
        }
    }
    Kirigami.OverlaySheet {
        id: addGroupSheet
        parent: root

        Kirigami.FormLayout {
            implicitWidth: Kirigami.Units.gridUnit * 15

            TextField {
                id: groupName
                Kirigami.FormData.label: i18n("Name:")
                placeholderText: i18n("Group Name")
            }
        }

        footer: RowLayout {
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: i18n("Ok")

                onClicked: {
                    if (groupName.text.length) {
                        kcm.imConfig.addGroup(groupName.text);
                        addGroupSheet.close();
                    }
                }
            }
        }
        header: Kirigami.Heading {
            text: i18n("Add Group")
        }
    }
    Connections {
        property int oldIndex: 0

        target: kcm

        function onCurrentIndexChanged(idx) {
            if (idx < oldIndex) {
                while (kcm.depth > idx + 1) {
                    var page = kcm.pageNeedsSave(kcm.depth - 1);
                    if (page === null) {
                        kcm.pop();
                    } else {
                        kcm.currentIndex = kcm.depth - 1;
                        page.showWarning();
                        break;
                    }
                }
            }
        }
        function onPagePushed() {
            oldIndex = kcm.depth;
        }
    }
    Dialog {
        id: confirmGroupChangeDialog
        property string nextGroup
        property string prevGroup

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        focus: true
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        title: i18n("Current group changed")
        x: (root.width - width) / 2
        y: root.height / 2 - height

        onAccepted: {
            kcm.imConfig.currentGroup = nextGroup;
        }
        onRejected: {
            var groups = kcm.imConfig.groups;
            for (var i = 0; i < groups.length; i++) {
                if (groups[i] == prevGroup) {
                    groupComboBox.currentIndex = i;
                    return;
                }
            }
        }

        Label {
            text: i18n("Do you want to change group? Changes to current group will be lost!")
        }

        Overlay.modal: Rectangle {
            color: "#99000000"
        }
    }

    footer: ColumnLayout {
        RowLayout {
            Button {
                icon.name: "input-keyboard"
                text: i18n("Select system layout...")

                onClicked: {
                    selectLayoutSheet.selectLayout(i18n("Select system layout for group %1", groupComboBox.currentText), "", kcm.imConfig.defaultLayout);
                }

                ToolTip {
                    text: i18n("Select system keyboard layout...")
                    visible: parent.hovered
                }
            }
            Label {
                text: kcm.layoutProvider.layoutDescription(kcm.imConfig.defaultLayout)
            }
            TextField {
                Layout.fillWidth: true
                placeholderText: i18n("Test Input")
            }
        }
        RowLayout {
            enabled: kcm.availability

            Button {
                icon.name: "configure"
                text: i18n("Configure global options...")

                onClicked: kcm.pushConfigPage(i18n("Global Options"), "fcitx://config/global")
            }
            Button {
                icon.name: "configure"
                text: i18n("Configure addons...")

                onClicked: kcm.push("AddonPage.qml")
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                icon.name: "list-add-symbolic"
                text: i18n("Add Input Method...")

                onClicked: kcm.push("AddIMPage.qml")
            }
        }
    }
    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: fcitxNotAvailableWarning
            Layout.fillWidth: true
            text: i18n("Cannot connect to Fcitx by DBus, is Fcitx running?")
            type: Kirigami.MessageType.Warning
            visible: !kcm.availability

            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    icon.name: "system-run"
                    text: i18n("Run Fcitx")

                    onTriggered: {
                        kcm.runFcitx();
                    }
                }
            ]
        }
        Kirigami.InlineMessage {
            id: fcitxNeedUpdateMessage
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("Found updates to fcitx installation. Do you want to check for newly installed input methods and addons? To update already loaded addons, fcitx would need to be restarted.")
            type: Kirigami.MessageType.Information
            visible: kcm.imConfig.needUpdate

            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    icon.name: "update-none"
                    text: i18n("Update")

                    onTriggered: {
                        kcm.imConfig.refresh();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    icon.name: "system-run"
                    text: i18n("Restart")

                    onTriggered: {
                        kcm.imConfig.restart();
                    }
                }
            ]
        }
        Kirigami.InlineMessage {
            id: layoutNotMatchWarning
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("Your currently configured input method does not match your layout, do you want to change the layout setting?")
            type: Kirigami.MessageType.Warning
            visible: false

            actions: [
                Kirigami.Action {
                    text: i18n("Fix")

                    onTriggered: {
                        kcm.fixLayout();
                        layoutNotMatchWarning.visible = false;
                    }
                }
            ]
        }
        Kirigami.InlineMessage {
            id: inputMethodNotMatchWarning
            Layout.fillWidth: true
            showCloseButton: true
            text: i18n("Your currently configured input method does not match your selected layout, do you want to add the corresponding input method for the layout?")
            type: Kirigami.MessageType.Warning
            visible: false

            actions: [
                Kirigami.Action {
                    text: i18n("Fix")

                    onTriggered: {
                        kcm.fixInputMethod();
                    }
                }
            ]
        }
        RowLayout {
            enabled: kcm.availability

            Label {
                text: i18n("Group:")
            }
            ComboBox {
                id: groupComboBox
                Layout.fillWidth: true
                model: kcm.imConfig.groups

                onActivated: {
                    if (kcm.imConfig.needSave && kcm.imConfig.currentGroup !== currentText && kcm.imConfig.currentGroup !== "") {
                        confirmGroupChangeDialog.nextGroup = currentText;
                        confirmGroupChangeDialog.prevGroup = kcm.imConfig.currentGroup;
                        confirmGroupChangeDialog.open();
                    } else {
                        kcm.imConfig.currentGroup = currentText;
                    }
                }
            }
            Button {
                icon.name: "list-add-symbolic"

                onClicked: {
                    groupName.text = "";
                    addGroupSheet.open();
                }
            }
            Button {
                icon.name: "list-remove-symbolic"
                visible: kcm.imConfig.groups && kcm.imConfig.groups.length > 1

                onClicked: {
                    kcm.imConfig.deleteGroup(groupComboBox.currentText);
                }
            }
        }
    }
    view: ListView {
        id: imList
        enabled: kcm.availability
        model: kcm.imConfig.currentIMModel

        section {
            property: "active"
            delegate: Kirigami.ListSectionHeader {
                text: section == "inactive" ? i18n("Input Method Off") : i18n("Input Method On")
            }
        }

        reuseItems: true

        moveDisplaced: Transition {
            YAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        delegate: delegateComponent
    }
}
