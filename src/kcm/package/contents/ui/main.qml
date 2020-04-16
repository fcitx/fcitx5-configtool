import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2 as KCM

KCM.ScrollViewKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 35
    implicitHeight: Kirigami.Units.gridUnit * 25

    Component {
        id: delegateComponent

        Kirigami.SwipeListItem {
            id: listItem

            actions: [
                Kirigami.Action {
                    iconName: "configure"
                    text: i18n("Configure")
                    visible: model !== null ? model.configurable : false
                    onTriggered: kcm.pushConfigPage(
                                     model.name,
                                     "fcitx://config/inputmethod/" + model.uniqueName)
                },
                Kirigami.Action {
                    iconName: "input-keyboard"
                    text: i18n("Select Layout")
                    visible: model !== null ? !model.uniqueName.startsWith(
                                                  "keyboard-") : false
                    onTriggered: selectLayoutSheet.selectLayout(
                                     model.uniqueName,
                                     (model.layout
                                      !== "" ? model.layout : kcm.imConfig.defaultLayout))
                },
                Kirigami.Action {
                    iconName: "list-remove"
                    text: i18n("Remove")
                    onTriggered: imList.model.remove(model.index)
                }
            ]

            RowLayout {
                Kirigami.ListItemDragHandle {
                    listItem: listItem
                    listView: imList
                    onMoveRequested: imList.model.move(oldIndex, newIndex, 1)
                }

                Label {
                    Layout.fillWidth: true
                    height: Math.max(implicitHeight,
                                     Kirigami.Units.iconSizes.smallMedium)
                    text: model !== null ? model.name : ""
                    color: listItem.checked
                           || (listItem.pressed && !listItem.checked
                               && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
                }
            }
        }
    }

    view: ListView {
        id: imList

        model: kcm.imConfig.currentIMModel
        moveDisplaced: Transition {
            YAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        delegate: Kirigami.DelegateRecycler {
            width: imList.width
            sourceComponent: delegateComponent
        }
    }

    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: fcitxNotAvailableWarning

            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            showCloseButton: true
            visible: false

            Connections {
                target: kcm
                onAvailabilityChanged: {
                    fcitxNotAvailableWarning.text = i18n(
                                "Cannot connect to Fcitx by DBus, is Fcitx running?")
                    fcitxNotAvailableWarning.visible = !avail
                }
            }
        }

        RowLayout {
            ComboBox {
                id: groupComboBox

                Layout.fillWidth: true
                model: kcm.imConfig.groups
                onActivated: {
                    if (kcm.imConfig.needSave
                            && kcm.imConfig.currentGroup !== currentText
                            && kcm.imConfig.currentGroup !== "") {
                        confirmGroupChangeDialog.nextGroup = currentText
                        confirmGroupChangeDialog.prevGroup = kcm.imConfig.currentGroup
                        confirmGroupChangeDialog.open()
                    } else {
                        kcm.imConfig.currentGroup = currentText
                    }
                }
            }
            Button {
                icon.name: "list-add"
                onClicked: {
                    groupName.text = ""
                    addGroupSheet.open()
                }
            }
            Button {
                icon.name: "list-remove"
                visible: kcm.imConfig.groups && kcm.imConfig.groups.length > 1
                onClicked: {
                    kcm.imConfig.deleteGroup(groupComboBox.currentText)
                }
            }
            Button {
                icon.name: "input-keyboard"
                onClicked: {
                    selectLayoutSheet.selectLayout("",
                                                   kcm.imConfig.defaultLayout)
                }

                ToolTip {
                    visible: parent.hovered
                    text: i18n("Select keyboard layout...")
                }
            }
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        Button {
            Layout.alignment: Qt.AlignRight
            text: i18n("Config global options...")
            icon.name: "configure"
            onClicked: kcm.pushConfigPage(i18n("Global Config"),
                                          "fcitx://config/global")
        }
        Button {
            Layout.alignment: Qt.AlignRight
            text: i18n("Config addons...")
            icon.name: "configure"
            onClicked: kcm.push("AddonPage.qml")
        }
        Button {
            Layout.alignment: Qt.AlignRight
            text: i18n("Add...")
            icon.name: "list-add"
            onClicked: kcm.push("AddIMPage.qml")
        }
    }

    SelectLayoutSheet {
        id: selectLayoutSheet

        parent: root
    }

    Kirigami.OverlaySheet {
        id: addGroupSheet

        parent: root
        header: Kirigami.Heading {
            text: i18n("Add Group")
        }
        footer: RowLayout {
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: i18n("Ok")
                onClicked: {
                    if (groupName.text.length) {
                        kcm.imConfig.addGroup(groupName.text)
                        addGroupSheet.close()
                    }
                }
            }
        }

        Kirigami.FormLayout {
            implicitWidth: Kirigami.Units.gridUnit * 15
            TextField {
                id: groupName
                Kirigami.FormData.label: i18n("Name:")
                placeholderText: i18n("Group Name")
            }
        }
    }

    Connections {
        target: kcm

        property int oldIndex: 0

        function onCurrentIndexChanged(idx) {
            if (idx < oldIndex) {
                while (kcm.depth > idx + 1) {
                    var page = kcm.pageNeedsSave(kcm.depth - 1)
                    if (page === null) {
                        kcm.pop()
                    } else {
                        kcm.currentIndex = kcm.depth - 1
                        page.showWarning()
                        break
                    }
                }
            }
        }

        function onPagePushed() {
            oldIndex = kcm.depth
        }
    }

    Dialog {
        id: confirmGroupChangeDialog

        property string prevGroup
        property string nextGroup

        title: i18n("Current group changed")
        standardButtons: Dialog.Yes | Dialog.No
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        modal: true
        focus: true
        x: (root.width - width) / 2
        y: root.height / 2 - height
        Overlay.modal: Rectangle {
            color: "#99000000"
        }
        onAccepted: {
            kcm.imConfig.currentGroup = nextGroup
        }
        onRejected: {
            var groups = kcm.imConfig.groups
            console.log(prevGroup)
            for (var i = 0; i < groups.length; i++) {
                if (groups[i] == prevGroup) {
                    groupComboBox.currentIndex = i
                    return
                }
            }
        }

        Label {
            text: i18n("Do you want to change group? Changes to current group will be lost!")
        }
    }
}
