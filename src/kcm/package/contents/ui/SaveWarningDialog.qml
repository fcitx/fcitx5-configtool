import QtQuick 2.14
import QtQuick.Controls 2.14

Dialog {
    id: notSavedDialog

    title: i18n("Some config is not saved")
    standardButtons: Dialog.Ok
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    modal: true
    focus: true
    x: (parent.width - width) / 2
    y: parent.height / 2 - height
    Overlay.modal: Rectangle {
        color: "#99000000"
    }

    Label {
        text: i18n("Current page is not yet saved. Please save the config first.")
    }
}
