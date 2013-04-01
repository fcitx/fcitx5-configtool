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
#include <QObject>
#include <QAbstractListModel>
#include <QStyledItemDelegate>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx/ui.h>

// self
#include "subconfigparser.h"
#include "skinpage.h"

enum FillRule {
    F_COPY = 0,
    F_RESIZE = 1
};


struct SkinPlacement {
    char *name;
    int x;
    int y;
};

struct SkinMainBar {
    char* backImg;
    char* eng;
    char* logo;
    char* active;
    int marginTop;
    int marginBottom;
    int marginLeft;
    int marginRight;
    FillRule fillV;
    FillRule fillH;
    char* placement;
};

struct SkinInputBar {
    int fontSize;
    FcitxConfigColor inputColor;
    FcitxConfigColor indexColor;
    FcitxConfigColor firstCandColor;
    FcitxConfigColor otherColor;
    boolean respectDPI;
    char* backImg;
    FcitxConfigColor cursorColor;
    int marginTop;
    int marginBottom;
    int marginLeft;
    int marginRight;
    char* backArrow;
    char* forwardArrow;
    int iBackArrowX;
    int iBackArrowY;
    int iForwardArrowX;
    int iForwardArrowY;
    int iInputPos;
    int iOutputPos;
    FillRule fillV;
    FillRule fillH;
};

struct SkinData {
    FcitxGenericConfig config;
    SkinMainBar mainbar;
    SkinInputBar inputbar;
};

namespace Fcitx
{
class SkinInfo
{
public:
    QString name;
    QString path;
    QPixmap pixmap;
};

class SkinPage::Private : public QObject
{
    enum {
        PixmapRole = 0x4532efd3,
        PathRole = 0x8F213873
    };

    Q_OBJECT
public:
    Private(QObject* parent = 0);
    virtual ~Private();

    class SkinModel;
    class SkinDelegate;

    SkinModel* skinModel;
    QListView* skinView;
    KPushButton* configureSkinButton;
    KPushButton* deleteSkinButton;
    SkinDelegate* skinDelegate;
    Module* module;
    KLineEdit* skinField;
public Q_SLOTS:
    void load();
    void deleteSkin();
    void configureSkin();
    void save();
    void currentSkinChanged();
Q_SIGNALS:
    void changed();
private:
    bool removeDir(const QString &dirName);
    SubConfigParser m_parser;
    SubConfig* m_subConfig;
};

class SkinPage::Private::SkinModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SkinModel(Private* d, QObject* parent = 0);
    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    void setSkinList(const QStringList& list);
    const QList<SkinInfo>& skinList() const;
private:
    QPixmap drawSkinPreview(const QString& path);
    void DrawResizableBackground(
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
    );
    void DrawWidget(
        QPixmap &destPixmap, QPixmap &widgetPixmap,
        int x, int y
    );
    QPixmap LoadImage(const char* skinDir, const char* fileName);
    QColor ConvertColor(FcitxConfigColor floatColor);
    Private* d;
    QList<SkinInfo> m_skins;
};

class SkinPage::Private::SkinDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SkinDelegate(QObject* parent = 0);
    ~SkinDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
signals:
    void regeneratePreview(const QModelIndex& index, const QSize& size) const;
};
}
