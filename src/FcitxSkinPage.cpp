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

#include "FcitxSkinPage.h"
#include <FcitxSkinPage_p.h>
#include <KNS3/DownloadDialog>
#include <KDebug>
#include <KStandardDirs>
#include <QSortFilterProxyModel>
#include <QPainter>
#include <QFile>
#include <fcitx-config/xdg.h>
#include "Module.h"
#include "ConfigDescManager.h"
#include "FcitxConfigPage.h"


CONFIG_BINDING_BEGIN(FcitxSkinInputBar)

CONFIG_BINDING_REGISTER("SkinFont", "FontSize", fontSize)

CONFIG_BINDING_REGISTER("SkinFont", "InputColor", inputColor)
CONFIG_BINDING_REGISTER("SkinFont", "IndexColor", indexColor)
CONFIG_BINDING_REGISTER("SkinFont", "FirstCandColor", firstCandColor)
CONFIG_BINDING_REGISTER("SkinFont", "OtherColor", otherColor)

CONFIG_BINDING_REGISTER("SkinInputBar", "BackImg", backImg)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginTop", marginTop)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginBottom", marginBottom)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginLeft", marginLeft)
CONFIG_BINDING_REGISTER("SkinInputBar", "MarginRight", marginRight)
CONFIG_BINDING_REGISTER("SkinInputBar", "CursorColor", cursorColor)
CONFIG_BINDING_REGISTER("SkinInputBar", "InputPos", iInputPos)
CONFIG_BINDING_REGISTER("SkinInputBar", "OutputPos", iOutputPos)
CONFIG_BINDING_REGISTER("SkinInputBar", "BackArrow", backArrow)
CONFIG_BINDING_REGISTER("SkinInputBar", "ForwardArrow", forwardArrow)
CONFIG_BINDING_REGISTER("SkinInputBar", "BackArrowX", iBackArrowX)
CONFIG_BINDING_REGISTER("SkinInputBar", "BackArrowY", iBackArrowY)
CONFIG_BINDING_REGISTER("SkinInputBar", "ForwardArrowX", iForwardArrowX)
CONFIG_BINDING_REGISTER("SkinInputBar", "ForwardArrowY", iForwardArrowY)
CONFIG_BINDING_REGISTER("SkinInputBar", "FillVertical", fillV)
CONFIG_BINDING_REGISTER("SkinInputBar", "FillHorizontal", fillH)

CONFIG_BINDING_REGISTER("SkinMainBar","BackImg", mainbar.backImg)
CONFIG_BINDING_REGISTER("SkinMainBar","Logo", mainbar.logo)
CONFIG_BINDING_REGISTER("SkinMainBar","Eng", mainbar.eng)
CONFIG_BINDING_REGISTER("SkinMainBar","Active", mainbar.active)
CONFIG_BINDING_REGISTER("SkinMainBar","MarginLeft", mainbar.marginLeft)
CONFIG_BINDING_REGISTER("SkinMainBar","MarginRight", mainbar.marginRight)
CONFIG_BINDING_REGISTER("SkinMainBar","MarginTop", mainbar.marginTop)
CONFIG_BINDING_REGISTER("SkinMainBar","MarginBottom", mainbar.marginBottom)
/*
CONFIG_BINDING_REGISTER("SkinTrayIcon","Active", )
CONFIG_BINDING_REGISTER("SkinTrayIcon","Inactive", )
*/

CONFIG_BINDING_END()


namespace Fcitx
{
    const int margin = 5;

    FcitxSkinPage::Private::SkinModel::SkinModel(Private* p, QObject* parent) :
            QAbstractListModel(parent),
            d(p)
    {
    }

    QVariant FcitxSkinPage::Private::SkinModel::data(const QModelIndex& index, int role ) const
    {
        FcitxSkinInfo* skin = static_cast<FcitxSkinInfo*>(index.internalPointer());
        switch (role)
        {
        case PixmapRole:
            return skin->pixmap;
        case PathRole:
            return skin->path;
        }
        return QVariant();
    }

    QModelIndex FcitxSkinPage::Private::SkinModel::index(int row, int column, const QModelIndex& parent) const
    {
        Q_UNUSED ( parent );

        return createIndex ( row, column, ( row < m_skins.count() ) ? ( void* ) &m_skins.at ( row ) : 0 );
    }

    int FcitxSkinPage::Private::SkinModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return m_skins.count();
    }

    bool FcitxSkinPage::Private::SkinModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        return QAbstractItemModel::setData(index, value, role);
    }

    const QList<FcitxSkinInfo>& FcitxSkinPage::Private::SkinModel::skinList() const
    {
        return m_skins;
    }

    void FcitxSkinPage::Private::SkinModel::setSkinList(const QStringList& list)
    {
        beginRemoveRows(QModelIndex(), 0, m_skins.size());
        m_skins.clear();
        endRemoveRows();

        QStringList sortedList = list;
        qSort(sortedList);

        Q_FOREACH(const QString& im, sortedList)
        {
            beginInsertRows(QModelIndex(), m_skins.size(), m_skins.size());
            FcitxSkinInfo info;
            info.path = im;
            info.pixmap = drawSkinPreview(im);
            m_skins.push_back(info);
            endInsertRows();
        }
    }

    QPixmap FcitxSkinPage::Private::SkinModel::drawSkinPreview(const QString& path)
    {
        ConfigFileDesc* cfdesc = d->module->configDescManager()->GetConfigDesc("skin.desc");
        FILE* fp = NULL;
        ConfigFile* cfile = NULL;
        QDir dir;
        if (cfdesc)
            fp = GetXDGFileWithPrefix("", path.toLocal8Bit().data(), "r", NULL);

        if (fp)
        {
            cfile = ParseConfigFileFp(fp, cfdesc);
            fclose(fp);
        }

        QString skinName = path.section('/', 1, 1);

        if (cfile)
        {
            QString skinDir = path.section('/', 0, 1);

            FcitxSkinInputBar inputbar;
            memset(&inputbar, 0, sizeof(FcitxSkinInputBar));
            FcitxSkinInputBarConfigBind(&inputbar, cfile, cfdesc);
            ConfigBindSync(&inputbar.config);

            int marginLeft=inputbar.marginLeft;
            int marginRight=inputbar.marginRight;
            int marginTop=inputbar.marginTop;
            int marginBottom=inputbar.marginBottom;

            // Draw Demo string:
            QString inputExample = skinName;
            QString numberStr[2];
            QString candStr[2];
            QString spaceStr=" ";
            for (int i=0; i<2; i++) {
                numberStr[i]=QString("%1.").arg(i+1);
            }
            candStr[0]=i18n("First candidate");
            candStr[1]=i18n("Other candidate");
            int offset=marginLeft;

            QFont inputFont(qApp->font());
            int fontHeight=inputbar.fontSize;
            inputFont.setPixelSize(fontHeight);
            QFontMetrics metrics(inputFont);

            // inputPos & outputPos is the LeftTop position of the text.
            int inputPos=marginTop+inputbar.iInputPos-fontHeight;
            int outputPos=marginTop+inputbar.iOutputPos-fontHeight;

            QPixmap inputBarPixmap = LoadImage(skinDir.toLocal8Bit().data(), inputbar.backImg);
            int resizeWidth=0;
            int resizeHeight=inputbar.iOutputPos;
            for (int i=0; i<2; i++) {
                resizeWidth+=metrics.width(numberStr[i]);
                resizeWidth+=metrics.width(candStr[i]);
            };
            int totalWidth=marginLeft + marginRight + resizeWidth;
            int totalHeight=marginTop + marginBottom + resizeHeight;

            QPixmap inputBarDestPixmap(totalWidth, totalHeight);
            inputBarDestPixmap.fill(Qt::transparent);
            DrawResizableBackground(inputBarDestPixmap, inputBarPixmap,
                                    marginLeft, marginRight, marginTop, marginBottom,
                                    resizeWidth, resizeHeight,
                                    inputbar.fillV, inputbar.fillH
                                );

            QPixmap backArrowPixmap = LoadImage(skinDir.toLocal8Bit().data(), inputbar.backArrow);
            QPixmap forwardArrowPixmap = LoadImage(skinDir.toLocal8Bit().data(), inputbar.forwardArrow);
            DrawWidget(inputBarDestPixmap, backArrowPixmap,
                    totalWidth - inputbar.iBackArrowX, inputbar.iBackArrowY
                    );
            DrawWidget(inputBarDestPixmap, forwardArrowPixmap,
                    totalWidth - inputbar.iForwardArrowX, inputbar.iForwardArrowY
                    );

            QPainter textPainter(&inputBarDestPixmap);
            textPainter.setFont(inputFont);

            QColor inputColor=ConvertColor(inputbar.inputColor);
            QColor indexColor=ConvertColor(inputbar.indexColor);
            QColor firstCandColor=ConvertColor(inputbar.firstCandColor);
            QColor otherColor=ConvertColor(inputbar.otherColor);

            textPainter.setPen(inputColor);
            textPainter.drawText(marginLeft, inputPos, metrics.width(inputExample), fontHeight, Qt::AlignVCenter, inputExample);

            // Draw candidate number:
            textPainter.setPen(indexColor);
            for (int i=0; i<2; i++) {
                textPainter.drawText(offset, outputPos, metrics.width(numberStr[i]), fontHeight, Qt::AlignVCenter, numberStr[i]);
                offset=offset + metrics.width(numberStr[i]) + metrics.width(candStr[i]) + metrics.width(spaceStr);
            }

            offset=marginLeft + metrics.width(numberStr[0]);

            textPainter.setPen(firstCandColor);
            textPainter.drawText(offset, outputPos, metrics.width(candStr[0]), fontHeight, Qt::AlignVCenter, candStr[0]);
            offset=offset+metrics.width(candStr[0]) + metrics.width(spaceStr) + metrics.width(numberStr[1]);

            textPainter.setPen(otherColor);
            textPainter.drawText(offset, outputPos, metrics.width(candStr[1]), fontHeight, Qt::AlignVCenter, candStr[1]);

            textPainter.end();

            /* FIXME:
             * Why this LoadImage cause error?
             * Binding problem?
             * FreeConfigFile also cause crash.
             * I don't understand the complex Bind system in Fcitx.
             *  -Ukyoi
             */

            /*Just define it for convenient:
             * mainbar->backImg == inputbar.mainbar.backImg;
             */
            FcitxSkinInputBar::FcitxSkinMainBar *mainbar=&inputbar.mainbar;

            QPixmap mainBarPixmap = LoadImage(skinDir.toLocal8Bit().data(), mainbar->backImg);
            int widgetCount=3;
            QPixmap mainBarWidgetPixmap[widgetCount];
            mainBarWidgetPixmap[0] = LoadImage(skinDir.toLocal8Bit().data(), mainbar->logo);
            mainBarWidgetPixmap[1] = LoadImage(skinDir.toLocal8Bit().data(), mainbar->eng);
            mainBarWidgetPixmap[2] = LoadImage(skinDir.toLocal8Bit().data(), mainbar->active);

            marginLeft=mainbar->marginLeft;
            marginRight=mainbar->marginRight;
            marginTop=mainbar->marginTop;
            marginBottom=mainbar->marginBottom;

            resizeWidth=0;
            resizeHeight=0;
            for (int i=0; i<widgetCount; i++) {
                resizeWidth+=mainBarWidgetPixmap[i].width();
                if (mainBarWidgetPixmap[i].height()>resizeHeight)
                    resizeHeight=mainBarWidgetPixmap[i].height();
            }

            QPixmap mainBarDestPixmap(marginLeft+resizeWidth+marginRight, marginTop+resizeHeight+marginBottom);
            DrawResizableBackground(mainBarDestPixmap, mainBarPixmap,
                                    marginLeft, marginRight, marginTop, marginBottom,
                                    resizeWidth, resizeHeight,
                                    F_RESIZE, F_RESIZE
                                );
            offset=marginLeft;
            for (int i=0; i<widgetCount; i++) {
                DrawWidget(mainBarDestPixmap, mainBarWidgetPixmap[i], offset, 0);
                offset+=mainBarWidgetPixmap[i].width();
            }

            // destPixmap应该是inputbar和mainbar的合体……
            QPixmap destPixmap( inputBarDestPixmap.width()+20+mainBarDestPixmap.width(), inputBarDestPixmap.height() );
            destPixmap.fill(Qt::transparent);
            DrawWidget(destPixmap, inputBarDestPixmap, 0, 0);
            DrawWidget(destPixmap, mainBarDestPixmap, inputBarDestPixmap.width()+20, inputBarDestPixmap.height()-mainBarDestPixmap.height());

            free(inputbar.backImg);
            free(inputbar.backArrow);
            free(inputbar.forwardArrow);

            free(mainbar->backImg);
            free(mainbar->active);
            free(mainbar->eng);
            free(mainbar->logo);

            FreeConfigFile(cfile);

            return destPixmap;
            // return inputBarDestPixmap;
        }
        else
        {
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

    QColor FcitxSkinPage::Private::SkinModel::ConvertColor(ConfigColor floatColor)
    {
        /**
        * 把浮点颜色转化成RGB整数颜色。
        */

        short r=(int)(floatColor.r*256);
        short g=(int)(floatColor.g*256);
        short b=(int)(floatColor.b*256);
        switch (r) {
            case 256 : r=255; break;
        }
        switch (g) {
            case 256 : g=255; break;
        }
        switch (b) {
            case 256 : b=255; break;
        }

        QColor converted(r, g, b);
        return converted;
    }

    QPixmap FcitxSkinPage::Private::SkinModel::LoadImage(const char* skinDir, const char* fileName)
    {
        char* image = NULL;
        FILE* fp = GetXDGFileWithPrefix(skinDir, fileName, "r", &image);
        QPixmap pixmap;
        if (fp)
        {
            fclose(fp);
            pixmap = QPixmap(QString::fromLocal8Bit(image));
        }

        if (image)
            free(image);

        return pixmap;
    }

    void FcitxSkinPage::Private::SkinModel::DrawWidget (
        QPixmap &destPixmap, QPixmap &widgetPixmap,
        int x, int y
    )
    {
        QPainter painter(&destPixmap);
        painter.drawPixmap( x, y, widgetPixmap );
        painter.end();
    }

    void FcitxSkinPage::Private::SkinModel::DrawResizableBackground (
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
        int originWidth=backgroundPixmap.width() - marginLeft - marginRight;
        int originHeight=backgroundPixmap.height() - marginTop - marginBottom;

        if ( resizeWidth <= 0 )
            resizeWidth = 1;
        if ( resizeHeight <= 0 )
            resizeHeight = 1;

        if ( originWidth <= 0 )
            originWidth = 1;
        if ( originHeight <= 0 )
            originHeight = 1;

        destPixmap=QPixmap(resizeWidth + marginLeft + marginRight, resizeHeight + marginTop + marginBottom);
        destPixmap.fill ( Qt::transparent );
        QPainter painter ( &destPixmap );


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

                    if (fillV == F_COPY)
                    {
                        if (j == repaintV - 1)
                        {
                            h = remainH;
                            oh = remainH;
                        }
                        else
                            h = originHeight;
                    }
                    else
                        h = resizeHeight;


                    if (fillH == F_COPY)
                    {
                        if (i == repaintH - 1)
                        {
                            w = remainW;
                            ow = remainW;
                        }
                        else
                            w = originWidth;
                    }
                    else
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

    FcitxSkinPage::Private::SkinDelegate::SkinDelegate(QObject* parent) :
            QStyledItemDelegate(parent)
    {
    }

    FcitxSkinPage::Private::SkinDelegate::~SkinDelegate()
    {
    }

    void FcitxSkinPage::Private::SkinDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        // highlight selected item
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

        QPixmap pixmap = index.model()->data(index, PixmapRole).value<QPixmap>();

        const QSize previewArea = option.rect.size() - QSize(2 * margin, 2 * margin);
        int offset = (previewArea.width() - pixmap.size().width()) / 2;
        painter->drawPixmap(option.rect.topLeft() + QPoint(margin + offset, margin), pixmap);
    }

    QSize FcitxSkinPage::Private::SkinDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        Q_UNUSED(option)
        QPixmap pixmap = index.model()->data(index, PixmapRole).value<QPixmap>();
        return pixmap.size() + QSize(2 * margin, 2 * margin);
    }

    FcitxSkinPage::Private::Private(QObject* parent) :
            QObject(parent),
            m_parser("Skin:configfile:skin/*/fcitx_skin.conf:skin.desc", this),
            m_subConfig(0)
    {
    }

    FcitxSkinPage::Private::~Private()
    {
        if (m_subConfig)
            delete m_subConfig;
    }

    void FcitxSkinPage::Private::load()
    {
        if (m_subConfig)
            delete m_subConfig;
        m_subConfig = m_parser.getSubConfig("Skin");
        skinModel->setSkinList(m_subConfig->filelist().toList());

        ConfigFileDesc* cfdesc = module->configDescManager()->GetConfigDesc("fcitx-classic-ui.desc");
        FILE* fp = NULL;
        ConfigFile* cfile = NULL;
        QString skinName;
        if (cfdesc)
            fp = GetXDGFileWithPrefix("conf", "fcitx-classic-ui.config", "r", NULL);
        if (fp)
        {
            cfile = ParseConfigFileFp(fp, cfdesc);
            fclose(fp);
        }
        if (cfile)
        {
            ConfigOption* option = ConfigFileGetOption(cfile, "ClassicUI", "SkinType");
            if (option)
                skinName = QString::fromUtf8(option->rawValue);
            FreeConfigFile(cfile);
        }

        int row = 0, currentSkin = -1;
        Q_FOREACH(const FcitxSkinInfo& skin, skinModel->skinList())
        {
            if (skin.path == QString("skin/%1/fcitx_skin.conf").arg(skinName))
            {
                currentSkin = row;
                break;
            }

            row ++;
        }

        if (currentSkin >= 0)
            skinView->selectionModel()->setCurrentIndex(skinModel->index(row, 0), QItemSelectionModel::ClearAndSelect);
    }

    void FcitxSkinPage::Private::save()
    {
        if (skinView->currentIndex().isValid())
        {
            QString skinName = skinView->currentIndex().data(PathRole).toString().section('/', 1, 1);

            ConfigFileDesc* cfdesc = module->configDescManager()->GetConfigDesc("fcitx-classic-ui.desc");
            FILE* fp = NULL;
            ConfigFile* cfile = NULL;
            if (cfdesc)
                fp = GetXDGFileWithPrefix("conf", "fcitx-classic-ui.config", "r", NULL);
            if (fp)
            {
                cfile = ParseConfigFileFp(fp, cfdesc);
                fclose(fp);
            }
            if (cfile)
            {
                ConfigOption* option = ConfigFileGetOption(cfile, "ClassicUI", "SkinType");
                if (option)
                {
                    if (option->rawValue)
                        free(option->rawValue);
                    option->rawValue = strdup(skinName.toUtf8().data());
                }
                GenericConfig gconfig;
                gconfig.configFile = cfile;
                fp = GetXDGFileUserWithPrefix("conf", "fcitx-classic-ui.config", "w", NULL);
                if (fp)
                {
                    SaveConfigFileFp(fp, &gconfig, cfdesc);
                    fclose(fp);
                }
                FreeConfigFile(cfile);
            }
        }
    }

    void FcitxSkinPage::Private::deleteSkin()
    {
        if (skinView->currentIndex().isValid())
        {
            FcitxSkinInfo* skin = static_cast<FcitxSkinInfo*> (skinView->currentIndex().internalPointer());
            char* path = NULL;
            FILE* fp = GetXDGFileWithPrefix ( "", skin->path.toLocal8Bit().data(), "r", &path );

            if (fp)
                fclose(fp);

            if (path)
            {
                QString p = QString::fromLocal8Bit(path);
                QFileInfo info(p);
                QDir dir = info.dir();
                removeDir(dir.absolutePath());
                free(path);
            }

            load();
        }
    }

    bool FcitxSkinPage::Private::removeDir(const QString &dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)) {
            Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                if (info.isDir()) {
                    result = removeDir(info.absoluteFilePath());
                }
                else {
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

    void FcitxSkinPage::Private::configureSkin()
    {
        if (skinView->currentIndex().isValid())
        {
            QItemSelectionModel* selectionModel = skinView->selectionModel();
            QModelIndex ind = selectionModel->currentIndex();
            if (!ind.isValid())
                return;
            FcitxSkinInfo* skin = static_cast<FcitxSkinInfo*>(ind.internalPointer());
            KDialog configDialog;
            ConfigDescManager manager;
            ConfigFileDesc* cfdesc = module->configDescManager()->GetConfigDesc("skin.desc");

            if ( cfdesc )
            {
                FcitxConfigPage* configPage = new FcitxConfigPage (
                    &configDialog,
                    cfdesc,
                    "",
                    skin->path
                );
                configDialog.setButtons ( KDialog::Ok | KDialog::Cancel | KDialog::Default );
                configDialog.setMainWidget ( configPage );
                connect ( &configDialog, SIGNAL ( buttonClicked ( KDialog::ButtonCode ) ), configPage, SLOT ( buttonClicked ( KDialog::ButtonCode ) ) );

                configDialog.exec();

                load();
            }
        }
    }

    void FcitxSkinPage::Private::currentSkinChanged()
    {
        if (skinView->currentIndex().isValid())
        {
            configureSkinButton->setEnabled(true);
            deleteSkinButton->setEnabled(true);
        }
        else
        {
            configureSkinButton->setEnabled(false);
            deleteSkinButton->setEnabled(false);
        }

        emit changed();
    }

    FcitxSkinPage::FcitxSkinPage(Module* module, QWidget* parent):
            QWidget(parent),
            m_module(module),
            d(new Private(this)),
            m_ui(new Ui::FcitxSkinPage)

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
        connect(d->skinView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), d, SLOT(currentSkinChanged()));
        connect(d, SIGNAL(changed()), this, SIGNAL(changed()));
    }

    FcitxSkinPage::~FcitxSkinPage()
    {
        delete m_ui;
    }

    void FcitxSkinPage::load()
    {
        if (NULL == d->module->configDescManager()->GetConfigDesc("fcitx-classic-ui.desc"))
        {
            this->setEnabled(false);
        }
        d->load();
    }

    void FcitxSkinPage::save()
    {
        d->save();
    }

    void FcitxSkinPage::installButtonClicked()
    {
        KNS3::DownloadDialog dialog("fcitx-skin.knsrc");
        dialog.exec();
        foreach (const KNS3::Entry& e, dialog.changedEntries()) {
            kDebug() << "Changed Entry: " << e.name();
        }
        load();
    }

}
