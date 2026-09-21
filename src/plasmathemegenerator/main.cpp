/*
 * SPDX-FileCopyrightText: 2022~2022 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "config.h"
#include "screenscaletracker.h"
#include <KIconLoader>
#include <KLocalizedString>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>
#include <Plasma/Theme>
#include <QBitmap>
#include <QColor>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLatin1String>
#include <QObject>
#include <QPainter>
#include <QSessionManager>
#include <QSocketNotifier>
#include <QStringLiteral>
#include <Qt>
#include <algorithm>
#include <cerrno>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/color.h>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/standardpaths.h>
#include <fcntl.h>
#include <memory>
#include <string>
using FrameSvg = KSvg::FrameSvg;
using Svg = KSvg::Svg;

namespace {
bool fd_is_valid(int fd) { return fcntl(fd, F_GETFD) != -1 || errno != EBADF; }

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

QString scaledImagePath(const QString &path, int scale) {
    if (scale == 1) {
        return path;
    }
    const QFileInfo fileInfo(path);
    return fileInfo.dir().filePath(QStringLiteral("%1@%2x.%3")
                                       .arg(fileInfo.completeBaseName())
                                       .arg(scale)
                                       .arg(fileInfo.suffix()));
}

template <typename ImageType>
bool safeSaveImage(const ImageType &image, const QString &path) {
    return fcitx::StandardPaths::global().safeSave(
        fcitx::StandardPathsType::Data, path.toLocal8Bit().constData(),
        [&image](int fd) {
            QFile file;
            if (!file.open(fd, QIODevice::WriteOnly)) {
                qDebug() << "FAILED TO OPEN QFILE";
                return false;
            }
            return image.save(&file, "png");
        });
}

} // namespace

class WatcherApp : public QGuiApplication {
    Q_OBJECT
public:
    WatcherApp(int &argc, char **argv) : QGuiApplication(argc, argv) {
        setApplicationName(QLatin1String("fcitx5-plasma-theme-generator"));
        setApplicationVersion(QLatin1String(PROJECT_VERSION));
    }

    bool init() {
        QCommandLineParser parser;
        parser.setApplicationDescription(
            i18n("Generate Fcitx 5 Classic UI Theme based on Plasma theme"));
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOptions(
            {{{"t", "theme"}, i18n("Plasma theme name <name> "), i18n("name")},
             {{"o", "output"}, i18n("Output path <output> "), i18n("output")}});
        QCommandLineOption option{"fd", i18n("File descriptor <fd> "),
                                  i18n("fd")};
        option.setFlags(QCommandLineOption::HiddenFromHelp);
        parser.addOption(option);
        parser.process(*this);

        if (parser.isSet("fd")) {
            int fd = -1;
            bool ok = false;
            fd = parser.value("fd").toInt(&ok);
            if (!ok || !fd_is_valid(fd)) {
                return false;
            }
            fd_ = fd;
        }

        if (parser.isSet("output")) {
            outputPath_ = parser.value("output");
        } else {
            outputPath_ = QString::fromStdString(
                (fcitx::StandardPaths::global().userDirectory(
                     fcitx::StandardPathsType::PkgData) /
                 "themes/plasma")
                    .string());
        }

        qDebug() << "Will write new themes to: " << outputPath_;

        if (parser.isSet("theme") && !monitorMode()) {
            theme_ = std::make_unique<Plasma::Theme>(parser.value("theme"));
        } else {
            theme_ = std::make_unique<Plasma::Theme>();
        }

        if (parser.isSet("theme") && !monitorMode()) {
            imageSet_ = std::make_unique<KSvg::ImageSet>(theme_->themeName());
        } else {
            // For monitor mode, we need a default constructed, so it can be
            // shared with global image set. It will be able to listen to
            // theme's global change like composite.
            imageSet_ = std::make_unique<KSvg::ImageSet>();
        }

        if (monitorMode()) {
            screenScaleTracker_ = new ScreenScaleTracker(this);
            socketNotifier_ =
                new QSocketNotifier(fd_, QSocketNotifier::Read, this);
            connect(socketNotifier_, &QSocketNotifier::activated, this,
                    [this]() {
                        char buf;
                        if (fcitx::fs::safeRead(fd_, &buf, 1) <= 0) {
                            quit();
                        }
                    });
            connect(theme_.get(), &Plasma::Theme::themeChanged, this, [this]() {
                auto selectors = imageSet_->selectors();
                // Force invalidate the cache to workaround a bug in ksvg.
                // FIXME: remove this once fix is merged.
                imageSet_->setSelectors({"bad"});
                imageSet_->setSelectors(selectors);
                if (!generateTheme()) {
                    qDebug() << "Failed to generate theme.";
                }
            });
            connect(screenScaleTracker_,
                    &ScreenScaleTracker::maximumScaleChanged, this,
                    [this](int) { regenerateThemeForScaleChange(); });
        }

        auto disableSessionManagement = [](QSessionManager &sm) {
            sm.setRestartHint(QSessionManager::RestartNever);
        };
        QObject::connect(this, &QGuiApplication::commitDataRequest,
                         disableSessionManagement);
        QObject::connect(this, &QGuiApplication::saveStateRequest,
                         disableSessionManagement);
        return true;
    }

    bool monitorMode() const { return fd_ >= 0; }

    bool generateTheme() {
        QDir dir(outputPath_);
        if (!dir.mkpath(".")) {
            return false;
        }
        const int maximumScale =
            screenScaleTracker_
                ? screenScaleTracker_->maximumScale()
                : ScreenScaleTracker::calculateMaximumScale(this);
        // Same logic from plasma-frameworks
        const int gridUnit = QFontMetrics(QGuiApplication::font())
                                 .boundingRect(QStringLiteral("M"))
                                 .height();
        const int smallSpacing =
            std::max(2, (gridUnit / 4)); // 1/4 of gridUnit, at least 2
        const qreal textMargin = smallSpacing / 2.0F;
        fcitx::RawConfig config;
        // Write metadata to theme config
        auto &metadata = config["Metadata"];
        metadata["Name"] = "Plasma";
        metadata["Version"] = "1";
        metadata["Author"] = "Fcitx";
        metadata["Description"] =
            i18n("Theme generated from Plasma Theme %1", theme_->themeName())
                .toStdString();
        config["SupportedScale"] = std::to_string(maximumScale);

        auto &inputPanel = config["InputPanel"];
        inputPanel["NormalColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();
        inputPanel["HighlightCandidateColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();
        inputPanel["HighlightColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::HighlightedTextColor))
                .toString();
        inputPanel["HighlightBackgroundColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::HighlightColor))
                .toString();
        inputPanel["PageButtonAlignment"] = "Last Candidate";

        inputPanel["NormalColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();
        inputPanel["HighlightCandidateColor"] =
            toFcitxColor(theme_->color(Plasma::Theme::TextColor)).toString();

        for (int scale = 1; scale <= maximumScale; scale++) {
            if (!renderScaleAssets(dir, config, textMargin, scale)) {
                return false;
            }
        }

        auto ret = fcitx::safeSaveAsIni(
            config, dir.filePath("theme.conf").toLocal8Bit().constData());

        if (monitorMode()) {
            char buf = 0;
            fcitx::fs::safeWrite(fd_, &buf, 1);
            qDebug() << "Notify theme reloading.";
        }
        if (ret) {
            generatedTheme_ = theme_->themeName();
            generatedMaximumScale_ = maximumScale;
        }
        return ret;
    }

    void regenerateThemeForScaleChange() {
        if (theme_->themeName() == generatedTheme_ &&
            screenScaleTracker_->maximumScale() <= generatedMaximumScale_) {
            return;
        }
        if (!generateTheme()) {
            qDebug() << "Failed to generate theme.";
        }
    }

    template <typename T>
    void setThemeToSvg(T &svg, int scale = 1) {
        svg.setImageSet(imageSet_.get());
        svg.setDevicePixelRatio(scale);
    }

private:
    bool renderScaleAssets(const QDir &dir, fcitx::RawConfig &config,
                           qreal textMargin, int scale) {
        auto &inputPanel = config["InputPanel"];
        auto &menu = config["Menu"];
        QImage background(QSize(200, 200) * scale, QImage::Format_ARGB32);
        background.setDevicePixelRatio(scale);
        background.fill(Qt::transparent);

        {
            qreal shadowLeft = 0;
            qreal shadowRight = 0;
            qreal shadowTop = 0;
            qreal shadowBottom = 0;
            qreal bgLeft = 0;
            qreal bgRight = 0;
            qreal bgTop = 0;
            qreal bgBottom = 0;
            FrameSvg shadowSvg;
            setThemeToSvg(shadowSvg, scale);
            shadowSvg.setImagePath("dialogs/background");
            const bool hasShadow = shadowSvg.hasElementPrefix("shadow");
            if (hasShadow) {
                shadowSvg.setElementPrefix("shadow");
                shadowSvg.resizeFrame(QSize(200, 200));
                shadowSvg.getMargins(shadowLeft, shadowTop, shadowRight,
                                     shadowBottom);
            }

            FrameSvg svg;
            setThemeToSvg(svg, scale);
            svg.setImagePath("dialogs/background");
            svg.resizeFrame(
                QSizeF(200, 200) -
                QSizeF(shadowLeft + shadowRight, shadowTop + shadowBottom));
            svg.getMargins(bgLeft, bgTop, bgRight, bgBottom);
            {
                QPainter p(&background);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.save();
                svg.paintFrame(&p, QPointF(shadowLeft, shadowTop));
                p.restore();
                if (hasShadow) {
                    p.save();
                    shadowSvg.paintFrame(&p);
                    p.restore();
                }
                p.end();
            }
            bgLeft += shadowLeft;
            bgTop += shadowTop;
            bgRight += shadowRight;
            bgBottom += shadowBottom;
            if (!safeSaveImage(
                    background,
                    scaledImagePath(dir.filePath("panel.png"), scale))) {
                return false;
            }
            svg.resizeFrame(
                QSizeF(200, 200) -
                QSizeF(shadowLeft + shadowRight, shadowTop + shadowBottom) -
                QSizeF(2, 2));
            if (theme_->blurBehindEnabled()) {
                QImage mask(QSize(200, 200) * scale, QImage::Format_ARGB32);
                mask.setDevicePixelRatio(scale);
                mask.fill(Qt::transparent);
                QPainter p(&mask);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.drawPixmap(QPointF(shadowLeft + 1, shadowTop + 1),
                             svg.alphaMask().mask());
                p.end();
                if (!safeSaveImage(
                        mask,
                        scaledImagePath(dir.filePath("mask.png"), scale))) {
                    return false;
                }
            }

            menu["Spacing"] = std::to_string(qRound(textMargin));
            setMarginsToConfig(inputPanel, "ContentMargin", bgLeft, bgTop,
                               bgRight, bgBottom);
            setMarginsToConfig(menu, "ContentMargin", bgLeft, bgTop, bgRight,
                               bgBottom);
            setMarginsToConfig(inputPanel, "ShadowMargin", shadowLeft,
                               shadowTop, shadowRight, shadowBottom);
            inputPanel["Background"]["Image"] = "panel.png";
            if (theme_->blurBehindEnabled()) {
                inputPanel["BlurMask"] = "mask.png";
                inputPanel["EnableBlur"] = "True";
            }
            menu["Background"]["Image"] = "panel.png";
            if (theme_->blurBehindEnabled()) {
                menu["BlurMask"] = "mask.png";
                menu["EnableBlur"] = "True";
            }
            setMarginsToConfig(inputPanel["Background"], "Margin", bgLeft,
                               bgTop, bgRight, bgBottom);
            setMarginsToConfig(menu["Background"], "Margin", bgLeft, bgTop,
                               bgRight, bgBottom);
        }

        {
            FrameSvg highlightSvg;
            setThemeToSvg(highlightSvg, scale);
            highlightSvg.setImagePath("widgets/viewitem");
            if (highlightSvg.hasElementPrefix("hover")) {
                highlightSvg.setElementPrefix("hover");
            } else if (highlightSvg.hasElementPrefix("selected")) {
                highlightSvg.setElementPrefix("selected");
            }
            highlightSvg.resizeFrame(QSize(200, 200));
            if (!safeSaveImage(
                    highlightSvg.framePixmap(),
                    scaledImagePath(dir.filePath("highlight.png"), scale))) {
                return false;
            }
            qreal bgLeft = 0;
            qreal bgRight = 0;
            qreal bgTop = 0;
            qreal bgBottom = 0;
            highlightSvg.getMargins(bgLeft, bgTop, bgRight, bgBottom);
            bgLeft = std::max(textMargin, bgLeft);
            bgTop = std::max(textMargin, bgTop);
            bgRight = std::max(textMargin, bgRight);
            bgBottom = std::max(textMargin, bgBottom);

            inputPanel["Highlight"]["Image"] = "highlight.png";
            menu["Highlight"]["Image"] = "highlight.png";
            setMarginsToConfig(inputPanel["Highlight"], "Margin", bgLeft, bgTop,
                               bgRight, bgBottom);
            setMarginsToConfig(menu["Highlight"], "Margin", bgLeft, bgTop,
                               bgRight, bgBottom);
            setMarginsToConfig(inputPanel, "TextMargin", bgLeft,
                               bgTop + textMargin, bgRight,
                               bgBottom + textMargin);
            setMarginsToConfig(menu, "TextMargin", bgLeft, bgTop, bgRight,
                               bgBottom);
        }

        {
            Svg icon;
            icon.setContainsMultipleImages(true);
            setThemeToSvg(icon, scale);
            icon.setImagePath("widgets/arrows");
            icon.resize(KIconLoader::SizeSmallMedium,
                        KIconLoader::SizeSmallMedium);
            if (icon.hasElement("left-arrow") &&
                icon.hasElement("right-arrow")) {
                inputPanel["PrevPage/Image"] = "prev.png";
                if (!safeSaveImage(
                        icon.pixmap("left-arrow"),
                        scaledImagePath(dir.filePath("prev.png"), scale))) {
                    return false;
                }
                inputPanel["NextPage/Image"] = "next.png";
                if (!safeSaveImage(
                        icon.pixmap("right-arrow"),
                        scaledImagePath(dir.filePath("next.png"), scale))) {
                    return false;
                }
            }
            icon.resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
            if (icon.hasElement("right-arrow")) {
                menu["SubMenu/Image"] = "arrow.png";
                if (!safeSaveImage(
                        icon.pixmap("right-arrow"),
                        scaledImagePath(dir.filePath("arrow.png"), scale))) {
                    return false;
                }
            }

            Svg radio;
            radio.setContainsMultipleImages(true);
            setThemeToSvg(radio, scale);
            radio.setImagePath("widgets/checkmarks");
            radio.resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
            if (radio.hasElement("radiobutton")) {
                menu["CheckBox/Image"] = "radio.png";
                if (!safeSaveImage(
                        radio.pixmap("radiobutton"),
                        scaledImagePath(dir.filePath("radio.png"), scale))) {
                    return false;
                }
            }
            Svg line;
            line.setContainsMultipleImages(true);
            setThemeToSvg(line, scale);
            line.setImagePath("widgets/line");
            if (line.hasElement("horizontal-line")) {
                if (!safeSaveImage(
                        line.pixmap("horizontal-line"),
                        scaledImagePath(dir.filePath("line.png"), scale))) {
                    return false;
                }
                menu["Separator/Image"] = "line.png";
            }
        }

        return true;
    }

    QSocketNotifier *socketNotifier_ = nullptr;
    int fd_ = -1;
    std::unique_ptr<Plasma::Theme> theme_;
    std::unique_ptr<KSvg::ImageSet> imageSet_;
    ScreenScaleTracker *screenScaleTracker_ = nullptr;
    QString outputPath_;
    QString generatedTheme_;
    int generatedMaximumScale_ = 0;
};

int main(int argc, char *argv[]) {
    WatcherApp app(argc, argv);
    if (!app.init()) {
        return 1;
    }

    auto ret = app.generateTheme();
    if (!app.monitorMode()) {
        return ret ? 0 : 1;
    }
    return app.exec();
}

#include "main.moc"
