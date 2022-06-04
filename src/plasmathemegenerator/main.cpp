/*
 * SPDX-FileCopyrightText: 2022~2022 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "config.h"
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/color.h>
#include <fcitx-utils/i18n.h>
#include <memory>
#include <qmath.h>

fcitx::Color toFcitxColor(const QColor &color) {
    fcitx::Color fcitxColor;
    fcitxColor.setRedF(color.redF());
    fcitxColor.setGreenF(color.greenF());
    fcitxColor.setBlueF(color.blueF());
    fcitxColor.setAlphaF(color.alphaF());
    return fcitxColor;
}

void setMarginsToConfig(fcitx::RawConfig &config, const std::string &name,
                        qreal left, qreal top, qreal right, qreal bottom) {
    auto &subConfig = config[name];
    subConfig["Left"] = std::to_string(qRound(left));
    subConfig["Top"] = std::to_string(qRound(top));
    subConfig["Right"] = std::to_string(qRound(right));
    subConfig["Bottom"] = std::to_string(qRound(bottom));
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QLatin1String("fcitx5-plasma-theme-generator"));
    app.setApplicationVersion(QLatin1String(PROJECT_VERSION));
    QCommandLineParser parser;
    parser.setApplicationDescription(
        _("Generatr Fcitx 5 Classic UI Theme based on Plasma theme"));
    parser.addHelpOption();
    parser.addOptions(
        {{{"t", "theme"}, _("Plasma theme name <name> "), _("name")},
         {{"o", "output"}, _("Output path <output> "), _("output")}});
    parser.process(app);

    QString outputPath;

    if (parser.isSet("output")) {
        outputPath = parser.value("output");
    } else {
        outputPath = QString::fromStdString(fcitx::stringutils::joinPath(
            fcitx::StandardPath::global().userDirectory(
                fcitx::StandardPath::Type::PkgData),
            "themes/plasma"));
    }

    qDebug() << "Will write new themes to: " << outputPath;

    QDir dir(outputPath);
    dir.mkpath(".");

    // Same logic from plasma-frameworks
    const int gridUnit = QFontMetrics(QGuiApplication::font())
                             .boundingRect(QStringLiteral("M"))
                             .height();
    const int smallSpacing =
        qMax(2, (gridUnit / 4)); // 1/4 of gridUnit, at least 2
    const qreal textMargin = smallSpacing / 2.0f;
    std::unique_ptr<Plasma::Theme> theme;
    if (parser.isSet("theme")) {
        theme = std::make_unique<Plasma::Theme>(parser.value("theme"));
    } else {
        theme = std::make_unique<Plasma::Theme>();
    }
    qDebug() << "Plasma Theme name:" << theme->themeName();
    fcitx::RawConfig config;
    // Write metadata to theme config
    auto &metadata = config["Metadata"];
    metadata["Name"] = "Plasma";
    metadata["Version"] = "1";
    metadata["Author"] = "Fcitx";
    metadata["Description"] = QString(_("Theme generated from Plasma Theme %1"))
                                  .arg(theme->themeName())
                                  .toStdString();

    auto &inputPanel = config["InputPanel"];
    inputPanel["NormalColor"] =
        toFcitxColor(theme->color(Plasma::Theme::TextColor)).toString();
    inputPanel["HighlightCandidateColor"] =
        toFcitxColor(theme->color(Plasma::Theme::TextColor)).toString();
    inputPanel["HighlightColor"] =
        toFcitxColor(theme->color(Plasma::Theme::HighlightedTextColor))
            .toString();
    inputPanel["HighlightBackgroundColor"] =
        toFcitxColor(theme->color(Plasma::Theme::HighlightColor)).toString();

    auto &menu = config["Menu"];
    inputPanel["NormalColor"] =
        toFcitxColor(theme->color(Plasma::Theme::TextColor)).toString();
    inputPanel["HighlightCandidateColor"] =
        toFcitxColor(theme->color(Plasma::Theme::TextColor)).toString();

    QImage background(QSize(200, 200), QImage::Format_ARGB32);
    background.fill(Qt::transparent);

    {
        qreal shadowLeft = 0, shadowRight = 0, shadowTop = 0, shadowBottom = 0;
        qreal bgLeft = 0, bgRight = 0, bgTop = 0, bgBottom = 0;
        Plasma::FrameSvg shadowSvg;
        shadowSvg.setTheme(theme.get());
        shadowSvg.setImagePath("dialogs/background");
        if (shadowSvg.hasElementPrefix("shadow")) {
            shadowSvg.setElementPrefix("shadow");
            shadowSvg.resizeFrame(QSize(200, 200));
            shadowSvg.getMargins(shadowLeft, shadowTop, shadowRight,
                                 shadowBottom);
        }

        Plasma::FrameSvg svg;
        svg.setTheme(theme.get());
        svg.setImagePath("dialogs/background");
        svg.resizeFrame(QSizeF(200, 200) - QSizeF(shadowLeft + shadowRight,
                                                  shadowTop + shadowBottom));
        svg.getMargins(bgLeft, bgTop, bgRight, bgBottom);
        {
            QPainter p(&background);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            svg.paintFrame(&p, QPointF(shadowLeft, shadowTop));
            shadowSvg.paintFrame(&p);
            p.end();
        }
        bgLeft += shadowLeft;
        bgTop += shadowTop;
        bgRight += shadowRight;
        bgBottom += shadowBottom;
        background.save(dir.filePath("panel.png"));

        menu["Spacing"] = std::to_string(textMargin);
        setMarginsToConfig(inputPanel, "ContentMargin", bgLeft, bgTop, bgRight,
                           bgBottom);
        setMarginsToConfig(menu, "ContentMargin", bgLeft, bgTop, bgRight,
                           bgBottom);
        setMarginsToConfig(inputPanel, "ShadowMargin", shadowLeft, shadowTop,
                           shadowRight, shadowBottom);
        inputPanel["Background"]["Image"] = "panel.png";
        menu["Background"]["Image"] = "panel.png";
        setMarginsToConfig(inputPanel["Background"], "Margin", bgLeft, bgTop,
                           bgRight, bgBottom);
        setMarginsToConfig(menu["Background"], "Margin", bgLeft, bgTop, bgRight,
                           bgBottom);
    }

    {
        Plasma::FrameSvg highlightSvg;
        highlightSvg.setTheme(theme.get());
        highlightSvg.setImagePath("widgets/viewitem");
        if (highlightSvg.hasElementPrefix("hover")) {
            highlightSvg.setElementPrefix("hover");
        } else if (highlightSvg.hasElementPrefix("selected")) {
            highlightSvg.setElementPrefix("selected");
        }
        highlightSvg.resizeFrame(QSize(200, 200));
        highlightSvg.framePixmap().save(dir.filePath("highlight.png"));
        qreal bgLeft = 0, bgRight = 0, bgTop = 0, bgBottom = 0;
        highlightSvg.getMargins(bgLeft, bgTop, bgRight, bgBottom);
        bgLeft = qMax(textMargin, bgLeft);
        bgTop = qMax(textMargin, bgTop);
        bgRight = qMax(textMargin, bgRight);
        bgBottom = qMax(textMargin, bgBottom);

        inputPanel["Highlight"]["Image"] = "highlight.png";
        menu["Highlight"]["Image"] = "highlight.png";
        setMarginsToConfig(inputPanel["Highlight"], "Margin", bgLeft, bgTop,
                           bgRight, bgBottom);
        setMarginsToConfig(menu["Highlight"], "Margin", bgLeft, bgTop, bgRight,
                           bgBottom);
        setMarginsToConfig(inputPanel, "TextMargin", bgLeft, bgTop + textMargin,
                           bgRight, bgBottom + textMargin);
        setMarginsToConfig(menu, "TextMargin", bgLeft, bgTop, bgRight,
                           bgBottom);
    }

    {
        Plasma::Svg icon;
        icon.setTheme(theme.get());
        icon.setImagePath("widgets/arrows");
        if (icon.hasElement("left-arrow") && icon.hasElement("right-arrow")) {
            inputPanel["PrevPage/Image"] = "prev.png";
            icon.pixmap("left-arrow").save(dir.filePath("prev.png"));
            inputPanel["NextPage/Image"] = "next.png";
            icon.pixmap("right-arrow").save(dir.filePath("next.png"));
        }
        if (icon.hasElement("right-arrow")) {
            menu["SubMenu/Image"] = "arrow.png";
            icon.pixmap("right-arrow").save(dir.filePath("arrow.png"));
        }

        Plasma::Svg radio;
        radio.setTheme(theme.get());
        radio.setImagePath("widgets/checkmarks");
        if (radio.hasElement("radiobutton")) {
            menu["CheckBox/Image"] = "radio.png";
            radio.pixmap("radiobutton").save(dir.filePath("radio.png"));
        }
        Plasma::Svg line;
        line.setTheme(theme.get());
        line.setImagePath("widgets/line");
        if (line.hasElement("horizontal-line")) {
            line.pixmap("horizontal-line").save(dir.filePath("line.png"));
            menu["Separator/Image"] = "line.png";
        }
    }

    return fcitx::safeSaveAsIni(config, fcitx::StandardPath::Type::PkgData,
                                "themes/plasma/theme.conf")
               ? 0
               : 1;
}
