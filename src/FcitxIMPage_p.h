#ifndef __FCITX_IM_PAGE_P_H__
#define __FCITX_IM_PAGE_P_H__
#include <QObject>
#include "FcitxIMPage.h"
#include <KCategorizedSortFilterProxyModel>
#include <KLocale>

class KCategorizedView;
class QListView;
class KPushButton;
class KLineEdit;
class KCategoryDrawerV3;
namespace Fcitx {
    
    class FcitxIMPage::Private
                : public QObject
    {
        Q_OBJECT
    public:
        Private(QObject* parent);
        virtual ~Private();
        void fetchIMList();
        const FcitxIMList& getIMList();

        class IMModel;
        
        class IMProxyModel;

        KPushButton* addIMButton;
        KPushButton* removeIMButton;
        KPushButton* moveUpButton;
        KPushButton* moveDownButton;
        QListView* currentIMView;
        KCategorizedView* availIMView;
        KLineEdit* filterTextEdit;
        
        IMModel* availIMModel;
        IMProxyModel* availIMProxyModel;
        KCategoryDrawerV3* categoryDrawer;
    
        IMModel* currentIMModel;
        
    Q_SIGNALS:
        void updateIMList();
        void changed();

    public Q_SLOTS:
        void availIMSelectionChanged();
        void currentIMCurrentChanged();
        void addIM();
        void removeIM();
        void moveUpIM();
        void moveDownIM();
        void save();
        
    private:
        QDBusConnection m_connection;
        org::fcitx::Fcitx::InputMethod* m_inputmethod;
        FcitxIMList m_list;
    };
    
    
    class FcitxIMPage::Private::IMProxyModel
        : public KCategorizedSortFilterProxyModel
    {
        Q_OBJECT

    public:
        IMProxyModel ( FcitxIMPage::Private *impage_d, QObject* parent = 0 );
        virtual ~IMProxyModel();

    protected:
        virtual bool filterAcceptsRow ( int source_row, const QModelIndex& source_parent ) const;
        virtual bool subSortLessThan ( const QModelIndex& left, const QModelIndex& right ) const;

    private:
        FcitxIMPage::Private* impage_d;
    };
    
    class FcitxIMPage::Private::IMModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
       
        IMModel ( FcitxIMPage::Private *impage_d, QObject* parent = 0 );
        virtual ~IMModel();

        virtual QModelIndex index ( int row, int column = 0, const QModelIndex& parent = QModelIndex() ) const;
        virtual QVariant data ( const QModelIndex& index, int role = Qt::DisplayRole ) const;
        virtual bool setData ( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
        virtual int rowCount ( const QModelIndex& parent = QModelIndex() ) const;
        
        void setShowOnlyEnabled( bool show );
    private Q_SLOTS:
        void filterIMEntryList();
    private:
        Private* impage_d;
        bool showOnlyEnabled;
        KLocale locale;
        FcitxIMList filteredIMEntryList;
    };
}

#endif
