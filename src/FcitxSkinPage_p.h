#include <QObject>
#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include "FcitxSubConfigParser.h"
#include "FcitxSkinPage.h"
#include <fcitx-config/fcitx-config.h>

enum FillRule {
    F_COPY = 0,
    F_RESIZE = 1
};

struct FcitxSkinInputBar
{
    GenericConfig config;
    int fontSize;
    ConfigColor inputColor;
    ConfigColor indexColor;
    ConfigColor firstCandColor;
    ConfigColor otherColor;
    char* backImg;
    ConfigColor cursorColor;
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

namespace Fcitx 
{
    class FcitxSkinInfo
    {
    public:
        QString path;
        QPixmap pixmap;
    };

    class FcitxSkinPage::Private : public QObject
    {
        enum
        {
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
        FcitxSubConfigParser m_parser;
        FcitxSubConfig* m_subConfig;
    };
    
    class FcitxSkinPage::Private::SkinModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        SkinModel(Private* d, QObject* parent = 0);
        virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
        void setSkinList(const QStringList& list);
        const QList<FcitxSkinInfo>& skinList() const;
    private:
        QPixmap drawSkinPreview(const QString& path);
        void DrawResizableBackground (
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
        void DrawWidget (
            QPixmap &destPixmap, QPixmap &widgetPixmap,
            int x, int y
        );
        QPixmap LoadImage(const char* skinDir, const char* fileName);
        QColor ConvertColor(ConfigColor floatColor);
        Private* d;
        QList<FcitxSkinInfo> m_skins;
    };
    
    class FcitxSkinPage::Private::SkinDelegate : public QStyledItemDelegate
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