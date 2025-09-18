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

Kirigami.OverlaySheet {
    id: selectLayoutSheet
    property string im: ""

    function selectLayout(title, im, layout) {
        heading.text = title;
        languageComboBox.currentIndex = 0;
        selectLayoutSheet.im = im;
        var layoutIndex = kcm.layoutProvider.layoutIndex(layout);
        layoutComboBox.currentIndex = layoutIndex;
        var variantIndex = kcm.layoutProvider.variantIndex(layout);
        variantComboBox.currentIndex = variantIndex;
        open();
    }

    Kirigami.FormLayout {
        implicitWidth: Kirigami.Units.gridUnit * 30

        ComboBox {
            id: languageComboBox
            Kirigami.FormData.label: i18n("Language:")
            implicitWidth: Kirigami.Units.gridUnit * 15
            model: kcm.layoutProvider.languageModel
            textRole: "name"

            onCurrentIndexChanged: {
                kcm.layoutProvider.layoutModel.language = model.language(currentIndex);
                layoutComboBox.currentIndex = 0;
                kcm.layoutProvider.setVariantInfo(kcm.layoutProvider.layoutModel.layoutInfo(layoutComboBox.currentIndex));
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
                    return;
                }
                kcm.layoutProvider.setVariantInfo(model.layoutInfo(currentIndex));
                variantComboBox.currentIndex = 0;
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

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        Button {
            text: i18n("Clear")
            visible: im !== ""

            onClicked: {
                kcm.imConfig.setLayout(im, "");
                selectLayoutSheet.close();
            }
        }
        Button {
            text: i18n("Ok")

            onClicked: {
                if (layoutComboBox.currentIndex >= 0 && variantComboBox.currentIndex >= 0) {
                    var layout = kcm.layoutProvider.layout(layoutComboBox.currentIndex, variantComboBox.currentIndex);
                    if (im.length === 0) {
                        kcm.imConfig.defaultLayout = layout;
                        if (kcm.imConfig.currentIMModel.count > 0 && "keyboard-%0".arg(layout) != kcm.imConfig.currentIMModel.imAt(0)) {
                            layoutNotMatchWarning.visible = false;
                            inputMethodNotMatchWarning.visible = true;
                        }
                    } else {
                        kcm.imConfig.setLayout(im, layout);
                    }
                    selectLayoutSheet.close();
                }
            }
        }
    }
    header: Kirigami.Heading {
        id: heading
    }
}
