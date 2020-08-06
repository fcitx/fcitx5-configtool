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

Kirigami.OverlaySheet {
    id: selectLayoutSheet

    property string im: ""

    function selectLayout(title, im, layout) {
        heading.text = title
        languageComboBox.currentIndex = 0
        selectLayoutSheet.im = im
        var layoutIndex = kcm.layoutProvider.layoutIndex(layout)
        layoutComboBox.currentIndex = layoutIndex
        var variantIndex = kcm.layoutProvider.variantIndex(layout)
        variantComboBox.currentIndex = variantIndex
        open()
    }

    header: Kirigami.Heading {
        id: heading
    }
    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        Button {
            visible: im !== ""
            text: i18n("Clear")
            onClicked: {
                kcm.imConfig.setLayout(im, "")
                selectLayoutSheet.close()
            }
        }
        Button {
            text: i18n("Ok")
            onClicked: {
                if (layoutComboBox.currentIndex >= 0
                        && variantComboBox.currentIndex >= 0) {
                    var layout = kcm.layoutProvider.layout(
                                layoutComboBox.currentIndex,
                                variantComboBox.currentIndex)
                    if (im.length === 0) {
                        kcm.imConfig.defaultLayout = layout
                        if (kcm.imConfig.currentIMModel.count > 0 &&
                            "keyboard-%0".arg(layout) != kcm.imConfig.currentIMModel.imAt(0)) {
                            layoutNotMatchWarning.visible = false;
                            inputMethodNotMatchWarning.visible = true;
                        }
                    } else {
                        kcm.imConfig.setLayout(im, layout)
                    }
                    selectLayoutSheet.close()
                }
            }
        }
    }

    Kirigami.FormLayout {
        ComboBox {
            id: languageComboBox

            Kirigami.FormData.label: i18n("Language:")
            implicitWidth: Kirigami.Units.gridUnit * 15
            model: kcm.layoutProvider.languageModel
            textRole: "name"
            onCurrentIndexChanged: {
                kcm.layoutProvider.layoutModel.language = model.language(
                            currentIndex);
                layoutComboBox.currentIndex = 0;
                kcm.layoutProvider.setVariantInfo(
                            kcm.layoutProvider.layoutModel.layoutInfo(
                                layoutComboBox.currentIndex));
                variantComboBox.currentIndex = 0;
            }
        }
        ComboBox {
            id: layoutComboBox

            Kirigami.FormData.label: i18n("Layout:")
            implicitWidth: Kirigami.Units.gridUnit * 15
            model: kcm.layoutProvider.layoutModel
            textRole: "name"
            onCurrentIndexChanged: {
                if (currentIndex < 0) {
                    return
                }
                kcm.layoutProvider.setVariantInfo(model.layoutInfo(
                                                      currentIndex))
                variantComboBox.currentIndex = 0
            }
        }
        ComboBox {
            id: variantComboBox

            Kirigami.FormData.label: i18n("Variant:")
            implicitWidth: Kirigami.Units.gridUnit * 15
            model: kcm.layoutProvider.variantModel
            textRole: "name"
        }
    }
}
