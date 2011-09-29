#include "FcitxIMPage.h"
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>
#include "FcitxIM.h"
#include "ui_FcitxIMPage.h"
#include "FcitxIMPage_p.h"
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>

namespace Fcitx
{
    FcitxIMPage::Private::IMModel::IMModel( FcitxIMPage::Private *d, QObject* parent )
        : QAbstractListModel(parent),
        impage_d(d),
        showOnlyEnabled(false),
        locale("kcm_fcitx")
    {
        connect(d, SIGNAL(updateIMList()), this, SLOT(filterIMEntryList()));
    }
    
    FcitxIMPage::Private::IMModel::~IMModel()
    {
    }

    QModelIndex FcitxIMPage::Private::IMModel::index ( int row, int column, const QModelIndex& parent ) const
    {
        Q_UNUSED ( parent );

        return createIndex ( row, column, ( row < filteredIMEntryList.count() ) ? ( void* ) &filteredIMEntryList.at ( row ) : 0 );
    }
    
    QVariant FcitxIMPage::Private::IMModel::data ( const QModelIndex& index, int role ) const
    {
        if ( !index.isValid() || !index.internalPointer() )
        {
            return QVariant();
        }

        FcitxIM *imEntry = static_cast<FcitxIM*> ( index.internalPointer() );

        switch ( role )
        {

        case Qt::DisplayRole:
            return imEntry->name();

        case Qt::DecorationRole:
            return QVariant();

        case KCategorizedSortFilterProxyModel::CategoryDisplayRole: // fall through

        case KCategorizedSortFilterProxyModel::CategorySortRole:
        {
            if (imEntry->langCode().isEmpty())
                return i18n("Unknown");
            else
                return locale.languageCodeToName(imEntry->langCode());
        }

        default:
            return QVariant();
        }
    }
    
    bool FcitxIMPage::Private::IMModel::setData ( const QModelIndex& index, const QVariant& value, int role )
    {
        return false;
    }
    
    int FcitxIMPage::Private::IMModel::rowCount ( const QModelIndex& parent ) const
    {
        if ( parent.isValid() )
        {
            return 0;
        }

        return filteredIMEntryList.count();
    }
    
    void FcitxIMPage::Private::IMModel::setShowOnlyEnabled( bool show )
    {
        if (show != showOnlyEnabled)
        {
            showOnlyEnabled = show;
            filterIMEntryList();
        }
    }
    
    void FcitxIMPage::Private::IMModel::filterIMEntryList()
    {
        FcitxIMList imEntryList = impage_d->getIMList();
        beginRemoveRows(QModelIndex(), 0, filteredIMEntryList.size());
        filteredIMEntryList.clear();
        endRemoveRows();
        Q_FOREACH(const FcitxIM& im, imEntryList)
        {
            if ((showOnlyEnabled && im.enabled()) || (!showOnlyEnabled && !im.enabled()))
            {
                beginInsertRows(QModelIndex(), filteredIMEntryList.size(), filteredIMEntryList.size());
                filteredIMEntryList.append(im);
                endInsertRows();
            }
        }
    }
    
    FcitxIMPage::Private::IMProxyModel::IMProxyModel ( FcitxIMPage::Private *d, QObject* parent )
        : KCategorizedSortFilterProxyModel(parent),
        impage_d(d)
    {
        
    }
    FcitxIMPage::Private::IMProxyModel::~IMProxyModel()
    {
    }

    bool FcitxIMPage::Private::IMProxyModel::filterAcceptsRow ( int source_row, const QModelIndex& source_parent ) const
    {
        Q_UNUSED ( source_parent )

        if ( !impage_d->filterTextEdit->text().isEmpty() )
        {
            const QModelIndex index = sourceModel()->index ( source_row, 0 );
            const FcitxIM* imEntry = static_cast<FcitxIM*> ( index.internalPointer() );
            return imEntry->name().contains ( impage_d->filterTextEdit->text(), Qt::CaseInsensitive )
            || imEntry->uniqueName().contains ( impage_d->filterTextEdit->text(), Qt::CaseInsensitive )
            || imEntry->langCode().contains ( impage_d->filterTextEdit->text(), Qt::CaseInsensitive );
        }
        return true;
    }

    bool FcitxIMPage::Private::IMProxyModel::subSortLessThan ( const QModelIndex& left, const QModelIndex& right ) const
    {
        return QString ( static_cast<FcitxIM*> ( left.internalPointer() )->name() ).compare ( ( QString ) ( static_cast<FcitxIM*> ( right.internalPointer() )->name() ), Qt::CaseInsensitive ) < 0;
    }
    
    
    FcitxIMPage::FcitxIMPage(QWidget* parent): QWidget(parent),
        m_ui(new Ui::FcitxIMPage),
        d(new Private(this))
    {        
        m_ui->setupUi(this);
        
        m_ui->addIMButton->setIcon(KIcon("go-next"));
        m_ui->removeIMButton->setIcon(KIcon("go-previous"));
        m_ui->moveUpButton->setIcon(KIcon("go-up"));
        m_ui->moveDownButton->setIcon(KIcon("go-down"));
        
        d->addIMButton = m_ui->addIMButton;
        d->removeIMButton = m_ui->removeIMButton;
        d->moveUpButton = m_ui->moveUpButton;
        d->moveDownButton = m_ui->moveDownButton;
        d->availIMView = m_ui->availIMView;
        d->currentIMView = m_ui->currentIMView;
        
        d->filterTextEdit = m_ui->filterTextEdit;
        
        d->filterTextEdit->setClearButtonShown ( true );
        d->filterTextEdit->setClickMessage ( i18n ( "Search Input Method" ) );
        d->availIMModel = new Private::IMModel(d, this);
        d->availIMProxyModel = new Private::IMProxyModel ( d, this );
        d->categoryDrawer = new KCategoryDrawerV3(d->availIMView);
        d->availIMView->setCategoryDrawer ( d->categoryDrawer );
        d->availIMProxyModel->setCategorizedModel ( true );
        d->availIMProxyModel->setSourceModel ( d->availIMModel );
        d->availIMView->setModel(d->availIMProxyModel);
        d->availIMView->setAlternatingBlockColors ( true );
        d->availIMView->setSelectionMode(QAbstractItemView::SingleSelection);
        
        d->currentIMModel = new Private::IMModel(d, this);
        d->currentIMModel->setShowOnlyEnabled(true);
        d->currentIMView->setModel(d->currentIMModel);
        d->currentIMView->setSelectionMode(QAbstractItemView::SingleSelection);
        
        connect ( d->filterTextEdit, SIGNAL ( textChanged ( QString ) ), d->availIMProxyModel, SLOT ( invalidate() ) );
        connect ( d->availIMView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), d, SLOT(availIMSelectionChanged()));
        connect ( d->currentIMView->selectionModel(), SIGNAL (currentChanged(QModelIndex,QModelIndex)), d, SLOT(currentIMCurrentChanged()));
        connect ( d->addIMButton, SIGNAL(clicked(bool)), d, SLOT(addIM()));
        connect ( d->removeIMButton, SIGNAL(clicked(bool)), d, SLOT(removeIM()));
        connect ( d->moveUpButton, SIGNAL(clicked(bool)), d, SLOT(moveUpIM()));
        connect ( d->moveDownButton, SIGNAL(clicked(bool)), d, SLOT(moveDownIM()));
        connect ( d, SIGNAL(changed()), this, SIGNAL(changed()));
        
        d->fetchIMList();
    }
    
    void FcitxIMPage::save()
    {
        d->save();
    }
    
    void FcitxIMPage::load()
    {
        d->fetchIMList();
    }
    
    FcitxIMPage::Private::Private(QObject* parent)
        : QObject(parent),
        availIMModel(0),
        m_connection(QDBusConnection::sessionBus())
    {
        m_inputmethod = new org::fcitx::Fcitx::InputMethod(
            QString("%1-%2").arg(FCITX_DBUS_SERVICE).arg(FcitxGetDisplayNumber()),
            FCITX_IM_DBUS_PATH,
            m_connection,
            this
        );
    }
    
    FcitxIMPage::Private::~Private()
    {
    }
    
    void FcitxIMPage::Private::availIMSelectionChanged()
    {
        if (!availIMView->selectionModel()->currentIndex().isValid())
            addIMButton->setEnabled(false);
        else
            addIMButton->setEnabled(true);
    }
    
    void FcitxIMPage::Private::currentIMCurrentChanged()
    {
        if (!currentIMView->selectionModel()->currentIndex().isValid())
        {
            removeIMButton->setEnabled(false);
            moveUpButton->setEnabled(false);
            moveDownButton->setEnabled(false);
        }
        else
        {
            if (currentIMView->selectionModel()->currentIndex().row() == 0)
                moveUpButton->setEnabled(false);
            else
                moveUpButton->setEnabled(true);
            if (currentIMView->selectionModel()->currentIndex().row() == currentIMModel->rowCount() - 1)
                moveDownButton->setEnabled(false);
            else
                moveDownButton->setEnabled(true);
            removeIMButton->setEnabled(true);
        }
    }
    
    void FcitxIMPage::Private::addIM()
    {
        if (availIMView->selectionModel()->currentIndex().isValid())
        {
            FcitxIM* im = static_cast<FcitxIM*>(availIMModel->index(availIMView->selectionModel()->currentIndex().row(), 0).internalPointer());
            int i = 0;
            for (i = 0; i < m_list.size(); i ++)
            {
                if (im->uniqueName() == m_list[i].uniqueName()) {
                    m_list[i].setEnabled(true);
                    qStableSort(m_list.begin(), m_list.end());
                    emit updateIMList();
                    emit changed();
                    break;
                }
            }
        }
    }
    
    void FcitxIMPage::Private::removeIM()
    {
        if (currentIMView->selectionModel()->currentIndex().isValid())
        {
            FcitxIM* im = static_cast<FcitxIM*>(currentIMView->selectionModel()->currentIndex().internalPointer());
            int i = 0;
            for (i = 0; i < m_list.size(); i ++)
            {
                if (im->uniqueName() == m_list[i].uniqueName()) {
                    m_list[i].setEnabled(false);
                    qStableSort(m_list.begin(), m_list.end());
                    emit updateIMList();
                    emit changed();
                    break;
                }
            }
        }
    }
    
    void FcitxIMPage::Private::moveDownIM()
    {
        QModelIndex curIndex = currentIMView->selectionModel()->currentIndex();
        if (curIndex.isValid())
        {
            QModelIndex nextIndex = currentIMModel->index(curIndex.row() + 1, 0);
            
            FcitxIM* curIM = static_cast<FcitxIM*>(curIndex.internalPointer());
            FcitxIM* nextIM = static_cast<FcitxIM*>(nextIndex.internalPointer());
            
            if (curIM == NULL || nextIM == NULL)
                return;
            
            int i = 0, curIMIdx = -1, nextIMIdx = -1;
            for (i = 0; i < m_list.size(); i ++)
            {
                if (curIM->uniqueName() == m_list[i].uniqueName())
                    curIMIdx = i;
                
                if (nextIM->uniqueName() == m_list[i].uniqueName())
                    nextIMIdx = i;
            }
            
            if (curIMIdx >= 0 && nextIMIdx >= 0 && curIMIdx != nextIMIdx)
            {
                m_list.swap(curIMIdx, nextIMIdx);
                qStableSort(m_list.begin(), m_list.end());
                emit updateIMList();
                emit changed();
            }
        }
    }
    
    void FcitxIMPage::Private::moveUpIM()
    {
        QModelIndex curIndex = currentIMView->selectionModel()->currentIndex();
        if (curIndex.isValid() && curIndex.row() > 0)
        {
            QModelIndex nextIndex = currentIMModel->index(curIndex.row() - 1, 0);
            
            FcitxIM* curIM = static_cast<FcitxIM*>(curIndex.internalPointer());
            FcitxIM* nextIM = static_cast<FcitxIM*>(nextIndex.internalPointer());
            
            if (curIM == NULL || nextIM == NULL)
                return;
            
            int i = 0, curIMIdx = -1, nextIMIdx = -1;
            for (i = 0; i < m_list.size(); i ++)
            {
                if (curIM->uniqueName() == m_list[i].uniqueName())
                    curIMIdx = i;
                
                if (nextIM->uniqueName() == m_list[i].uniqueName())
                    nextIMIdx = i;
            }
            
            if (curIMIdx >= 0 && nextIMIdx >= 0 && curIMIdx != nextIMIdx)
            {
                m_list.swap(curIMIdx, nextIMIdx);
                qStableSort(m_list.begin(), m_list.end());
                emit updateIMList();
                emit changed();
            }
        }
    }
    
    void FcitxIMPage::Private::save()
    {
        if (m_inputmethod->isValid())
            m_inputmethod->setIMList(m_list);
    }
    
    void FcitxIMPage::Private::fetchIMList()
    {
        if (m_inputmethod->isValid())
        {
            m_list = m_inputmethod->iMList();
            qStableSort(m_list.begin(), m_list.end());
            emit updateIMList();
        }
    }
       
    const FcitxIMList& FcitxIMPage::Private::getIMList()
    {
        return m_list;
    }
    
    FcitxIMPage::~FcitxIMPage()
    {
        delete m_ui;
    }

}