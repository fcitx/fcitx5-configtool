/***************************************************************************
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/


// Qt
#include <QSortFilterProxyModel>
#include <QPainter>
#include <QFile>
#include <QPointer>

// KDE
#include <KDebug>
#include <KStandardDirs>
#include <KNS3/DownloadDialog>

// Fcitx
#include <fcitx-config/xdg.h>

// self
#include "module.h"
#include "global.h"
#include "configwidget.h"
#include "skinpage.h"
#include "skinpage_p.h"


CONFIG_BINDING_BEGIN(SkinData)

CONFIG_BINDING_REGISTER("SkinFont", "FontSize", inputbar.fontSize)

CONFIG_BINDING_REGISTER("SkinFont", "InputColor", inputbar.inputColor)
CONFIG_BINDING_REGISTER("SkinFont", "IndexColor", inputbar.indexColor)
CONFIG_BINDING_REGISTER("SkinFont", "FirstCandColor", inputbar.firstCandColor)
CONFIG_BINDING_REGISTER("SkinFont", "OtherColor", inputbar.otherColor)
CONFIG_BINDING_REGISTER("SkinFont", "RespectDPI", inputbar.respectDPI)

CONFIG_BINDING_REGISTER("SkinInputBar", "BackImg", inputbar.backImg)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginTop", inputbar.marginTop)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginBottom", inputbar.marginBottom)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginLeft", inputbar.marginLeft)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginRight", inputbar.marginRight)
CONFIG_BINDING_REGISTER("SkinInputBar", "CursorColor", inputbar.cursorColor)
CONFIG_BINDING_REGISTER("SkinInputBar", "InputPos", inputbar.iInputPos)
CONFIG_BINDING_REGISTER("SkinInputBar", "OutputPos", inputbar.iOutputPos)
CONFIG_BINDING_REGISTER("SkinInputBar", "BackArrow", inputbar.backArrow)
CONFIG_BINDING_REGISTER("SkinInputBar", "ForwardArrow", inputbar.forwardArrow)
CONFIG_BINDING_REGISTER("SkinInputBar", "BackArrowX", inputbar.iBackArrowX)
CONFIG_BINDING_REGISTER("SkinInputBar", "BackArrowY", inputbar.iBackArrowY)
CONFIG_BINDING_REGISTER("SkinInputBar", "ForwardArrowX", inputbar.iForwardArrowX)
CONFIG_BINDING_REGISTER("SkinInputBar", "ForwardArrowY", inputbar.iForwardArrowY)
CONFIG_BINDING_REGISTER("SkinInputBar", "FillVertical", inputbar.fillV)
CONFIG_BINDING_REGISTER("SkinInputBar", "FillHorizontal", inputbar.fillH)

CONFIG_BINDING_REGISTER("SkinMainBar", "BackImg", mainbar.backImg)
CONFIG_BINDING_REGISTER("SkinMainBar", "Logo", mainbar.logo)
CONFIG_BINDING_REGISTER("SkinMainBar", "Eng", mainbar.eng)
CONFIG_BINDING_REGISTER("SkinMainBar", "Active", mainbar.active)
CONFIG_BINDING_REGISTER("SkinMainBar", "MarginLeft", mainbar.marginLeft)
CONFIG_BINDING_REGISTER("SkinMainBar", "MarginRight", mainbar.marginRight)
CONFIG_BINDING_REGISTER("SkinMainBar", "MarginTop", mainbar.marginTop)
CONFIG_BINDING_REGISTER("SkinMainBar", "MarginBottom", mainbar.marginBottom)
CONFIG_BINDING_REGISTER("SkinMainBar", "FillVertical", mainbar.fillV)
CONFIG_BINDING_REGISTER("SkinMainBar", "FillHorizontal", mainbar.fillH)
CONFIG_BINDING_REGISTER("SkinMainBar", "Placement", mainbar.placement)
/*
CONFIG_BINDING_REGISTER("SkinTrayIcon","Active", )
CONFIG_BINDING_REGISTER("SkinTrayIcon","Inactive", )
*/

CONFIG_BINDING_END()

static inline void FreePlacement(void* arg) {
    SkinPlacement* sp = (SkinPlacement*) arg;
    free(sp->name);
}

namespace Fcitx
{
const int margin = 5;

static const UT_icd place_icd = {sizeof(SkinPlacement), NULL, NULL, FreePlacement};

void ParsePlacement(UT_array* sps, char* placment)
{
    UT_array* array = fcitx_utils_split_string(placment, ';');
    char** str;
    utarray_clear(sps);
    for (str = (char**) utarray_front(array);
            str != NULL;
            str = (char**) utarray_next(array, str)) {
        char* s = *str;
        char* p = strchr(s, ':');
        if (p == NULL)
            continue;
        int len = p - s;
        SkinPlacement sp;
        sp.name = strndup(s, len);
        int ret = sscanf(p + 1, "%d,%d", &sp.x, &sp.y);
        if (ret != 2)
            continue;
        utarray_push_back(sps, &sp);
    }

    fcitx_utils_free_string_list(array);
}

SkinPage::Private::SkinModel::SkinModel(Private* p, QObject* parent) :
    QAbstractListModel(parent),
    d(p)
{
}

QVariant SkinPage::Private::SkinModel::data(const QModelIndex& index, int role) const
{
    SkinInfo* skin = static_cast<SkinInfo*>(index.internalPointer());
    switch (role) {
    case PixmapRole:
        return skin->pixmap;
    case PathRole:
        return skin->path;
    }
    return QVariant();
}

QModelIndex SkinPage::Private::SkinModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, (row < m_skins.count()) ? (void*) &m_skins.at(row) : 0);
}

int SkinPage::Private::SkinModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_skins.count();
}

bool SkinPage::Private::SkinModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return QAbstractItemModel::setData(index, value, role);
}

const QList<SkinInfo>& SkinPage::Private::SkinModel::skinList() const
{
    return m_skins;
}

void SkinPage::Private::SkinModel::setSkinList(const QStringList& list)
{
    beginRemoveRows(QModelIndex(), 0, m_skins.size());
    m_skins.clear();
    endRemoveRows();

    QStringList sortedList = list;
    qSort(sortedList);

    Q_FOREACH(const QString & im, sortedList) {
        beginInsertRows(QModelIndex(), m_skins.size(), m_skins.size());
        SkinInfo info;
        info.path = im;
        info.pixmap = drawSkinPreview(im);
        m_skins.push_back(info);
        endInsertRows();
    }
}

QPixmap SkinPage::Private::SkinModel::drawSkinPreview(const QString& path)
{

    FcitxConfigFileDesc* cfdesc = Global::instance()->GetConfigDesc("skin.desc");
    FILE* fp = NULL;
    FcitxConfigFile* cfile = NULL;
    QDir dir;
    if (cfdesc)
        fp = FcitxXDGGetFileWithPrefix("", path.toLocal8Bit().constData(), "r", NULL);

    if (fp) {
        cfile = FcitxConfigParseConfigFileFp(fp, cfdesc);
        fclose(fp);
    }

    QString skinName = path.section('/', 1, 1);

    if (cfile) {
        QString skinDir = path.section('/', 0, 1);

        SkinData skin;
        memset(&skin, 0, sizeof(SkinData));
        SkinDataConfigBind(&skin, cfile, cfdesc);
        FcitxConfigBindSync(&skin.config);

        int marginLeft = skin.inputbar.marginLeft;
        int marginRight = skin.inputbar.marginRight;
        int marginTop = skin.inputbar.marginTop;
        int marginBottom = skin.inputbar.marginBottom;

        // Draw Demo string:
        QString inputExample = skinName;
        QString numberStr[2];
        QString candStr[2];
        QString spaceStr = " ";
        for (int i = 0; i < 2; i++) {
            numberStr[i] = QString("%1.").arg(i + 1);
        }
        candStr[0] = i18n("First candidate");
        candStr[1] = i18n("Other candidate");
        int offset = marginLeft;

        QFont inputFont(qApp->font());
        int fontHeight = 0;
        if (skin.inputbar.respectDPI) {
            inputFont.setPointSize(fontHeight);
            QFontMetrics metrics(inputFont);
            fontHeight = metrics.height();
        }
        else {
            inputFont.setPixelSize(skin.inputbar.fontSize);
            fontHeight = skin.inputbar.fontSize;
        }
        QFontMetrics metrics(inputFont);

        // inputPos & outputPos is the LeftTop position of the text.
        int inputPos;
        if (skin.inputbar.respectDPI)
            inputPos = marginTop + skin.inputbar.iInputPos;
        else
            inputPos = marginTop + skin.inputbar.iInputPos - fontHeight;
        int outputPos;
        if (skin.inputbar.respectDPI)
            outputPos = marginTop + skin.inputbar.iInputPos + fontHeight + skin.inputbar.iOutputPos;
        else
            outputPos = marginTop + skin.inputbar.iOutputPos - fontHeight;

        QPixmap inputBarPixmap = LoadImage(skinDir.toLocal8Bit().constData(), skin.inputbar.backImg);
        int resizeWidth = 0;
        int resizeHeight;
        if (skin.inputbar.respectDPI)
            resizeHeight = skin.inputbar.iInputPos + skin.inputbar.iOutputPos + fontHeight * 2;
        else
            resizeHeight = skin.inputbar.iOutputPos;
        for (int i = 0; i < 2; i++) {
            resizeWidth += metrics.width(numberStr[i]);
            resizeWidth += metrics.width(candStr[i]);
        };
        int totalWidth = marginLeft + marginRight + resizeWidth;
        int totalHeight = marginTop + marginBottom + resizeHeight;

        QPixmap inputBarDestPixmap(totalWidth, totalHeight);
        inputBarDestPixmap.fill(Qt::transparent);
        DrawResizableBackground(inputBarDestPixmap, inputBarPixmap,
                                marginLeft, marginRight, marginTop, marginBottom,
                                resizeWidth, resizeHeight,
                                skin.inputbar.fillV, skin.inputbar.fillH
                               );

        QPixmap backArrowPixmap = LoadImage(skinDir.toLocal8Bit().constData(), skin.inputbar.backArrow);
        QPixmap forwardArrowPixmap = LoadImage(skinDir.toLocal8Bit().constData(), skin.inputbar.forwardArrow);
        DrawWidget(inputBarDestPixmap, backArrowPixmap,
                   totalWidth - skin.inputbar.iBackArrowX, skin.inputbar.iBackArrowY
                  );
        DrawWidget(inputBarDestPixmap, forwardArrowPixmap,
                   totalWidth - skin.inputbar.iForwardArrowX, skin.inputbar.iForwardArrowY
                  );

        QPainter textPainter(&inputBarDestPixmap);
        textPainter.setFont(inputFont);

        QColor inputColor = ConvertColor(skin.inputbar.inputColor);
        QColor indexColor = ConvertColor(skin.inputbar.indexColor);
        QColor firstCandColor = ConvertColor(skin.inputbar.firstCandColor);
        QColor otherColor = ConvertColor(skin.inputbar.otherColor);

        textPainter.setPen(inputColor);
        textPainter.drawText(marginLeft, inputPos, metrics.width(inputExample), fontHeight, Qt::AlignVCenter, inputExample);

        // Draw candidate number:
        textPainter.setPen(indexColor);
        for (int i = 0; i < 2; i++) {
            textPainter.drawText(offset, outputPos, metrics.width(numberStr[i]), fontHeight, Qt::AlignVCenter, numberStr[i]);
            offset = offset + metrics.width(numberStr[i]) + metrics.width(candStr[i]) + metrics.width(spaceStr);
        }

        offset = marginLeft + metrics.width(numberStr[0]);

        textPainter.setPen(firstCandColor);
        textPainter.drawText(offset, outputPos, metrics.width(candStr[0]), fontHeight, Qt::AlignVCenter, candStr[0]);
        offset = offset + metrics.width(candStr[0]) + metrics.width(spaceStr) + metrics.width(numberStr[1]);

        textPainter.setPen(otherColor);
        textPainter.drawText(offset, outputPos, metrics.width(candStr[1]), fontHeight, Qt::AlignVCenter, candStr[1]);

        textPainter.end();

        /*Just define it for convenient:
         * mainbar->backImg == inputbar.mainbar.backImg;
         */
        SkinMainBar *mainbar = &skin.mainbar;

        UT_array placement;
        utarray_init(&placement, &place_icd);
        ParsePlacement(&placement, mainbar->placement);

        QPixmap mainBarPixmap = LoadImage(skinDir.toLocal8Bit().constData(), mainbar->backImg);
        const int widgetCount = 2;
        QPixmap mainBarWidgetPixmap[widgetCount];
        mainBarWidgetPixmap[0] = LoadImage(skinDir.toLocal8Bit().constData(), mainbar->logo);
        mainBarWidgetPixmap[1] = LoadImage(skinDir.toLocal8Bit().constData(), mainbar->active);

        marginLeft = mainbar->marginLeft;
        marginRight = mainbar->marginRight;
        marginTop = mainbar->marginTop;
        marginBottom = mainbar->marginBottom;

        QPixmap mainBarDestPixmap;

        if (utarray_len(&placement) != 0) {
            mainBarDestPixmap = QPixmap(mainBarPixmap.width(), mainBarPixmap.height());
            DrawResizableBackground(mainBarDestPixmap, mainBarPixmap,
                                    0, 0, 0, 0,
                                    mainBarPixmap.width(), mainBarPixmap.height(),
                                    F_RESIZE, F_RESIZE
                                   );
            SkinPlacement* sp;
            for (sp = (SkinPlacement*) utarray_front(&placement);
                    sp != NULL;
                    sp = (SkinPlacement*) utarray_next(&placement, sp)) {

                if (strcmp(sp->name, "logo") == 0) {
                    DrawWidget(mainBarDestPixmap, mainBarWidgetPixmap[0], sp->x, sp->y);
                } else if (strcmp(sp->name, "im") == 0) {
                    DrawWidget(mainBarDestPixmap, mainBarWidgetPixmap[1], sp->x, sp->y);
                }
            }
        } else {
            resizeWidth = 0;
            resizeHeight = 0;
            for (int i = 0; i < widgetCount; i++) {
                resizeWidth += mainBarWidgetPixmap[i].width();
                if (mainBarWidgetPixmap[i].height() > resizeHeight)
                    resizeHeight = mainBarWidgetPixmap[i].height();
            }

            mainBarDestPixmap = QPixmap(marginLeft + resizeWidth + marginRight, marginTop + resizeHeight + marginBottom);
            DrawResizableBackground(mainBarDestPixmap, mainBarPixmap,
                                    marginLeft, marginRight, marginTop, marginBottom,
                                    resizeWidth, resizeHeight,
                                    mainbar->fillV, mainbar->fillH
                                   );
            offset = marginLeft;
            for (int i = 0; i < widgetCount; i++) {
                DrawWidget(mainBarDestPixmap, mainBarWidgetPixmap[i], offset, marginTop);
                offset += mainBarWidgetPixmap[i].width();
            }
        }

        // destPixmap includes both inputbar and mainbar
        int maxHeight = qMax(mainBarDestPixmap.height(),
                             inputBarDestPixmap.height());
        QPixmap destPixmap(inputBarDestPixmap.width() + 20 +
                           mainBarDestPixmap.width(), maxHeight);
        destPixmap.fill(Qt::transparent);
        DrawWidget(destPixmap, inputBarDestPixmap, 0, 0);
        DrawWidget(destPixmap, mainBarDestPixmap,
                   inputBarDestPixmap.width() + 20,
                   maxHeight - mainBarDestPixmap.height());

        FcitxConfigFree(&skin.config);
        utarray_done(&placement);

        return destPixmap;
        // return inputBarDestPixmap;
    } else {
        QFont inputFont(qApp->font());
        QFontMetrics fm(inputFont);
        QString errmsg = i18n("Skin %1 Cannot be loaded").arg(skinName);
        int w = fm.width(errmsg);
        QPixmap destPixmap(w, fm.height());
        destPixmap.fill(Qt::transparent);

        QPainter textPainter(&destPixmap);
        textPainter.setPen(qApp->palette().color(QPalette::Active, QPalette::Text));
        textPainter.setFont(inputFont);
        textPainter.drawText(0, 0, w, fm.height(), Qt::AlignVCenter, errmsg);
        textPainter.end();

        return destPixmap;
    }
}

QColor SkinPage::Private::SkinModel::ConvertColor(FcitxConfigColor floatColor)
{
    /**
     * 0-1 to 0-255
     */
    return QColor(qBound(0, int(floatColor.r * 256), 255),
                  qBound(0, int(floatColor.g * 256), 255),
                  qBound(0, int(floatColor.b * 256), 255));
}

QPixmap SkinPage::Private::SkinModel::LoadImage(const char* skinDir, const char* fileName)
{
    char* image = NULL;
    FILE* fp = FcitxXDGGetFileWithPrefix(skinDir, fileName, "r", &image);
    QPixmap pixmap;
    if (fp) {
        fclose(fp);
        pixmap = QPixmap(QString::fromLocal8Bit(image));
    }

    if (image)
        free(image);

    return pixmap;
}

void SkinPage::Private::SkinModel::DrawWidget(
    QPixmap &destPixmap, QPixmap &widgetPixmap,
    int x, int y
)
{
    QPainter painter(&destPixmap);
    painter.drawPixmap(x, y, widgetPixmap);
    painter.end();
}

void SkinPage::Private::SkinModel::DrawResizableBackground(
    QPixmap &destPixmap,
    QPixmap &backgroundPixmap,
    int marginLeft,
    int marginRight,
    int marginTop,
    int marginBottom,
    int resizeWidth,
    int resizeHeight,
    FillRule fillV,
    FillRule fillH
)
{
    int originWidth = backgroundPixmap.width() - marginLeft - marginRight;
    int originHeight = backgroundPixmap.height() - marginTop - marginBottom;

    if (resizeWidth <= 0)
        resizeWidth = 1;
    if (resizeHeight <= 0)
        resizeHeight = 1;

    if (originWidth <= 0)
        originWidth = 1;
    if (originHeight <= 0)
        originHeight = 1;

    destPixmap = QPixmap(resizeWidth + marginLeft + marginRight, resizeHeight + marginTop + marginBottom);
    destPixmap.fill(Qt::transparent);
    QPainter painter(&destPixmap);


    /* 画背景 */

    /* 九宫格
    * 7 8 9
    * 4 5 6
    * 1 2 3
    */
    /* part 1 */
    painter.drawPixmap(
        QRect(0, marginTop + resizeHeight, marginLeft, marginBottom),
        backgroundPixmap,
        QRect(0, marginTop + originHeight, marginLeft, marginBottom)
    );

    /* part 3 */
    painter.drawPixmap(
        QRect(marginLeft + resizeWidth, marginTop + resizeHeight, marginRight, marginBottom),
        backgroundPixmap,
        QRect(marginLeft + originWidth, marginTop + originHeight, marginRight, marginBottom)
    );

    /* part 7 */
    painter.drawPixmap(
        QRect(0 , 0, marginLeft, marginTop),
        backgroundPixmap,
        QRect(0, 0, marginLeft, marginTop)
    );

    /* part 9 */
    painter.drawPixmap(
        QRect(marginLeft + resizeWidth, 0, marginRight, marginTop),
        backgroundPixmap,
        QRect(marginLeft + originWidth, 0, marginRight, marginTop)
    );


    /* part 2 & 8 */
    {
        if (fillH == F_COPY) {
            int repaint_times = resizeWidth / originWidth;
            int remain_width = resizeWidth % originWidth;
            int i = 0;

            for (i = 0; i < repaint_times; i++) {
                painter.drawPixmap(
                    QRect(marginLeft + i * originWidth, 0, originWidth, marginTop),
                    backgroundPixmap,
                    QRect(marginLeft, 0, originWidth, marginTop)
                );
                painter.drawPixmap(
                    QRect(marginLeft + i * originWidth, marginTop + resizeHeight, originWidth, marginBottom),
                    backgroundPixmap,
                    QRect(marginLeft, marginTop + originHeight, originWidth, marginBottom)
                );
            }

            if (remain_width != 0) {
                painter.drawPixmap(
                    QRect(marginLeft + repaint_times * originWidth, 0, remain_width, marginTop),
                    backgroundPixmap,
                    QRect(marginLeft, 0, remain_width, marginTop)
                );
                painter.drawPixmap(
                    QRect(marginLeft + repaint_times * originWidth, marginTop + resizeHeight, remain_width, marginBottom),
                    backgroundPixmap,
                    QRect(marginLeft, marginTop + originHeight, remain_width, marginBottom)
                );
            }
        } else {
            painter.drawPixmap(
                QRect(marginLeft, 0, resizeWidth, marginTop),
                backgroundPixmap,
                QRect(marginLeft, 0, originWidth, marginTop)
            );
            painter.drawPixmap(
                QRect(marginLeft, marginTop + resizeHeight, resizeWidth, marginBottom),
                backgroundPixmap,
                QRect(marginLeft, marginTop + originHeight, originWidth, marginBottom)
            );
        }
    }

    /* part 4 & 6 */
    {
        if (fillV == F_COPY) {
            int repaint_times = resizeHeight / originHeight;
            int remain_height = resizeHeight % originHeight;
            int i = 0;

            for (i = 0; i < repaint_times; i++) {
                painter.drawPixmap(
                    QRect(0, marginTop + i * originHeight , marginLeft, originHeight),
                    backgroundPixmap,
                    QRect(0, marginTop , marginLeft, originHeight)
                );

                painter.drawPixmap(
                    QRect(marginLeft + resizeWidth, marginTop + i * originHeight , marginRight, originHeight),
                    backgroundPixmap,
                    QRect(marginLeft + originWidth, marginTop , marginRight, originHeight)
                );
            }

            if (remain_height != 0) {
                painter.drawPixmap(
                    QRect(0, marginTop + repaint_times * originHeight , marginLeft, remain_height),
                    backgroundPixmap,
                    QRect(0, marginTop , marginLeft, remain_height)
                );

                painter.drawPixmap(
                    QRect(marginLeft + resizeWidth, marginTop + repaint_times * originHeight , marginRight, remain_height),
                    backgroundPixmap,
                    QRect(marginLeft + originWidth, marginTop , marginRight, remain_height)
                );
            }
        } else {
            painter.drawPixmap(
                QRect(0, marginTop , marginLeft, resizeHeight),
                backgroundPixmap,
                QRect(0, marginTop , marginLeft, originHeight)
            );

            painter.drawPixmap(
                QRect(marginLeft + resizeWidth, marginTop , marginRight, resizeHeight),
                backgroundPixmap,
                QRect(marginLeft + originWidth, marginTop , marginRight, originHeight)
            );
        }
    }

    /* part 5 */
    {
        int repaintH = 0, repaintV = 0;
        int remainW = 0, remainH = 0;

        if (fillH == F_COPY) {
            repaintH = resizeWidth / originWidth + 1;
            remainW = resizeWidth % originWidth;
        } else {
            repaintH = 1;
        }

        if (fillV == F_COPY) {
            repaintV = resizeHeight / originHeight + 1;
            remainH = resizeHeight % originHeight;
        } else {
            repaintV = 1;
        }


        int i, j;
        for (i = 0; i < repaintH; i ++) {
            for (j = 0; j < repaintV; j ++) {
                int ow = originWidth, oh = originHeight, w, h;

                if (fillV == F_COPY) {
                    if (j == repaintV - 1) {
                        h = remainH;
                        oh = remainH;
                    } else
                        h = originHeight;
                } else
                    h = resizeHeight;


                if (fillH == F_COPY) {
                    if (i == repaintH - 1) {
                        w = remainW;
                        ow = remainW;
                    } else
                        w = originWidth;
                } else
                    w = resizeWidth;
                painter.drawPixmap(
                    QRect(marginLeft + i * originWidth, marginTop + j * originHeight , w, h),
                    backgroundPixmap,
                    QRect(marginLeft, marginTop , ow, oh)
                );
            }
        }
    }
    painter.end();
}

SkinPage::Private::SkinDelegate::SkinDelegate(QObject* parent) :
    QStyledItemDelegate(parent)
{
}

SkinPage::Private::SkinDelegate::~SkinDelegate()
{
}

void SkinPage::Private::SkinDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // highlight selected item
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    QPixmap pixmap = index.model()->data(index, PixmapRole).value<QPixmap>();

    const QSize previewArea = option.rect.size() - QSize(2 * margin, 2 * margin);
    int offset = (previewArea.width() - pixmap.size().width()) / 2;
    painter->drawPixmap(option.rect.topLeft() + QPoint(margin + offset, margin), pixmap);
}

QSize SkinPage::Private::SkinDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    QPixmap pixmap = index.model()->data(index, PixmapRole).value<QPixmap>();
    return pixmap.size() + QSize(2 * margin, 2 * margin);
}

SkinPage::Private::Private(QObject* parent) :
    QObject(parent),
    m_parser("Skin:configfile:skin/*/fcitx_skin.conf:skin.desc", this),
    m_subConfig(0)
{
}

SkinPage::Private::~Private()
{
    if (m_subConfig)
        delete m_subConfig;
}

void SkinPage::Private::load()
{
    if (!skinField)
        return;

    if (m_subConfig)
        delete m_subConfig;

    m_subConfig = m_parser.getSubConfig("Skin");
    skinModel->setSkinList(m_subConfig->fileList().toList());

    QString skinName = skinField->text();

    int row = 0, currentSkin = -1;
    Q_FOREACH(const SkinInfo & skin, skinModel->skinList()) {
        if (skin.path == QString("skin/%1/fcitx_skin.conf").arg(skinName)) {
            currentSkin = row;
            break;
        }

        row ++;
    }

    if (currentSkin >= 0)
        skinView->selectionModel()->setCurrentIndex(skinModel->index(row, 0), QItemSelectionModel::ClearAndSelect);
}

void SkinPage::Private::save()
{
}

void SkinPage::Private::deleteSkin()
{
    if (skinView->currentIndex().isValid()) {
        SkinInfo* skin = static_cast<SkinInfo*>(skinView->currentIndex().internalPointer());
        char* path = NULL;
        FILE* fp = FcitxXDGGetFileWithPrefix("", skin->path.toLocal8Bit().constData(), "r", &path);

        if (fp)
            fclose(fp);

        if (path) {
            QString p = QString::fromLocal8Bit(path);
            QFileInfo info(p);
            QDir dir = info.dir();
            removeDir(dir.absolutePath());
            free(path);
        }

        load();
    }
}

bool SkinPage::Private::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

void SkinPage::Private::configureSkin()
{
    if (skinView->currentIndex().isValid()) {
        QItemSelectionModel* selectionModel = skinView->selectionModel();
        QModelIndex ind = selectionModel->currentIndex();
        if (!ind.isValid())
            return;
        SkinInfo* skin = static_cast<SkinInfo*>(ind.internalPointer());
        FcitxConfigFileDesc* cfdesc = Global::instance()->GetConfigDesc("skin.desc");

        if (cfdesc) {
            QPointer<KDialog> configDialog(ConfigWidget::configDialog(
                module,
                cfdesc,
                "",
                skin->path
            ));

            configDialog->exec();
            delete configDialog;

            load();
        }
    }
}

void SkinPage::Private::currentSkinChanged()
{
    if (skinView->currentIndex().isValid()) {
        configureSkinButton->setEnabled(true);
        deleteSkinButton->setEnabled(true);

        if (skinField) {
            QString skinName = skinView->currentIndex().data(PathRole).toString().section('/', 1, 1);
            skinField->setText(skinName);
        }
    } else {
        configureSkinButton->setEnabled(false);
        deleteSkinButton->setEnabled(false);
    }

    emit changed();
}

SkinPage::SkinPage(Module* module, QWidget* parent):
    QWidget(parent),
    m_module(module),
    d(new Private(this)),
    m_ui(new Ui::SkinPage)

{
    m_ui->setupUi(this);
    m_ui->installSkinButton->setIcon(KIcon("get-hot-new-stuff"));

    d->configureSkinButton = m_ui->configureSkinButton;
    d->deleteSkinButton = m_ui->deleteSkinButton;
    d->configureSkinButton->setIcon(KIcon("configure"));
    d->skinView = m_ui->skinView;
    d->skinModel = new Private::SkinModel(d, this);
    d->skinDelegate = new Private::SkinDelegate(this);
    d->skinView->setModel(d->skinModel);
    d->skinView->setItemDelegate(d->skinDelegate);
    d->module = module;

    connect(m_ui->installSkinButton, SIGNAL(clicked()), this, SLOT(installButtonClicked()));
    connect(d->deleteSkinButton, SIGNAL(clicked(bool)), d, SLOT(deleteSkin()));
    connect(d->configureSkinButton, SIGNAL(clicked(bool)), d, SLOT(configureSkin()));
    connect(d->skinView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), d, SLOT(currentSkinChanged()));
    connect(d, SIGNAL(changed()), this, SIGNAL(changed()));
}

SkinPage::~SkinPage()
{
    delete m_ui;
}

void SkinPage::load()
{
    if (NULL == Global::instance()->GetConfigDesc("fcitx-classic-ui.desc")) {
        this->setEnabled(false);
    }
    d->load();
}

void SkinPage::save()
{
    d->save();
}

void SkinPage::setSkinField(KLineEdit* lineEdit)
{
    d->skinField = lineEdit;

    setEnabled(d->skinField != 0);

    if (d->skinField) {
        load();
    }
}

void SkinPage::installButtonClicked()
{
    QPointer<KNS3::DownloadDialog> dialog(new KNS3::DownloadDialog("fcitx-skin.knsrc"));
    dialog->exec();
    foreach(const KNS3::Entry & e, dialog->changedEntries()) {
        kDebug() << "Changed Entry: " << e.name();
    }
    delete dialog;
    load();
}

}
