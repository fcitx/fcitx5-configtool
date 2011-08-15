#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>
#include <QPainter>
#include <KPushButton>
#include <KLineEdit>
#include <KCategorizedView>
#include <KCategoryDrawer>
#include <klocalizedstring.h>
#include <kdebug.h>
#include <libintl.h>
#include <fcitx-config/fcitx-config.h>
#include "FcitxAddonSelector.h"
#include "FcitxAddonSelector_p.h"
#include "FcitxConfigPage.h"
#include "Module.h"
#include "ConfigDescManager.h"

#define MARGIN 5

FcitxAddonSelector::Private::Private(FcitxAddonSelector* parent) :
        QObject(parent),
        listView(0),
        categoryDrawer(0),
        parent(parent)
{
}

FcitxAddonSelector::Private::~Private()
{
}

int FcitxAddonSelector::Private::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (listView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }
 
    return totalWidth - width - value;
}

FcitxAddonSelector::Private::AddonModel::AddonModel(FcitxAddonSelector::Private *addonSelector_d, QObject* parent)
        : QAbstractListModel(parent),
        addonSelector_d(addonSelector_d)
{
}

FcitxAddonSelector::Private::AddonModel::~AddonModel()
{
}

QModelIndex FcitxAddonSelector::Private::AddonModel::index(int row, int column, const QModelIndex& parent ) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, (row < addonEntryList.count()) ? (void*) addonEntryList.at(row) : 0 );
}

QVariant FcitxAddonSelector::Private::AddonModel::data(const QModelIndex& index, int role) const
{
    {
        if (!index.isValid() || !index.internalPointer()) {
            return QVariant();
        }

        FcitxAddon *addonEntry = static_cast<FcitxAddon*>(index.internalPointer());

        switch (role) {
        case Qt::DisplayRole:
            return QString::fromUtf8(addonEntry->generalname);
        case CommentRole:
            return QString::fromUtf8(addonEntry->comment);
        case ConfigurableRole:
            {
                ConfigFileDesc* cfdesc = this->addonSelector_d->parent->parent->configDescManager()->GetConfigDesc(QString::fromUtf8(addonEntry->name).append(".desc"));
                return (bool)(cfdesc != NULL);
            }
        case Qt::DecorationRole:
            return QVariant();
        case Qt::CheckStateRole:
            return addonEntry->bEnabled;
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole: // fall through
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            {
                const ConfigOptionDesc *codesc = ConfigDescGetOptionDesc(addonEntry->config.configFile->fileDesc, "Addon", "Category");
                const ConfigEnum *e = &codesc->configEnum;
                return QString::fromUtf8(dgettext("fcitx", e->enumDesc[addonEntry->category]));
            }
        default:
            return QVariant();
        }
    }
}

bool FcitxAddonSelector::Private::AddonModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        static_cast<FcitxAddon*>(index.internalPointer())->bEnabled = value.toBool();
        ret = true;
    }

    if (ret) {
        emit dataChanged(index, index);
    }

    return ret;
}

void FcitxAddonSelector::Private::AddonModel::addAddon(FcitxAddon* addon)
{
    beginInsertRows(QModelIndex(), addonEntryList.count(), addonEntryList.count());
    addonEntryList << addon;
    endInsertRows();
}

int FcitxAddonSelector::Private::AddonModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return addonEntryList.count();
}

 FcitxAddonSelector::Private::ProxyModel::ProxyModel(FcitxAddonSelector::Private *addonSelector_d, QObject *parent)
     : KCategorizedSortFilterProxyModel(parent)
     , addonSelector_d(addonSelector_d)
 {
     sort(0);
 }
 
 FcitxAddonSelector::Private::ProxyModel::~ProxyModel()
 {
 }
 
 bool FcitxAddonSelector::Private::ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
 {
     Q_UNUSED(sourceParent)
 
     if (!addonSelector_d->lineEdit->text().isEmpty()) {
         const QModelIndex index = sourceModel()->index(sourceRow, 0);
         const FcitxAddon* addonInfo = static_cast<FcitxAddon*>(index.internalPointer());
         return QString(addonInfo->name).contains(addonSelector_d->lineEdit->text(), Qt::CaseInsensitive);
     }
 
     return true;
 }
 
bool FcitxAddonSelector::Private::ProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return QString(static_cast<FcitxAddon*>(left.internalPointer())->name).compare((QString)(static_cast<FcitxAddon*>(right.internalPointer())->name), Qt::CaseInsensitive) < 0;
}

FcitxAddonSelector::Private::AddonDelegate::AddonDelegate(FcitxAddonSelector::Private *addonSelector_d, QObject *parent)
    : KWidgetItemDelegate(addonSelector_d->listView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new KPushButton)
    , addonSelector_d(addonSelector_d)
{
    pushButton->setIcon(KIcon("configure")); // only for getting size matters
}

FcitxAddonSelector::Private::AddonDelegate::~AddonDelegate()
{
    delete checkBox;
    delete pushButton;
}

void FcitxAddonSelector::Private::AddonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }
 
    int xOffset = checkBox->sizeHint().width();
 
    painter->save();
 
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
 
    QRect contentsRect(addonSelector_d->dependantLayoutValue(MARGIN * 2 + option.rect.left() + xOffset, option.rect.width() - MARGIN * 2 - xOffset, option.rect.width()), MARGIN + option.rect.top(), option.rect.width() - MARGIN * 2 - xOffset, option.rect.height() - MARGIN * 2);
 
    int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();
 
    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);
 
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }
 
    if (addonSelector_d->listView->layoutDirection() == Qt::RightToLeft) {
        contentsRect.translate(lessHorizontalSpace, 0);
    }
 
    painter->save();
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();
 
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(index.model()->data(index, CommentRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();
}
 
QSize FcitxAddonSelector::Private::AddonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 4;
    int j = 1;
  
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
 
    return QSize(fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()) +
                       0 + MARGIN * i + pushButton->sizeHint().width() * j,
                 fmTitle.height() + option.fontMetrics.height() + MARGIN * 2);
}
 
QList<QWidget*> FcitxAddonSelector::Private::AddonDelegate::createItemWidgets() const
{
    QList<QWidget*> widgetList;
 
    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(slotStateChanged(bool)));
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(emitChanged()));
 
    KPushButton *configurePushButton = new KPushButton;
    configurePushButton->setIcon(KIcon("configure"));
    connect(configurePushButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureClicked()));
 
    setBlockedEventTypes(enabledCheckBox, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                            << QEvent::KeyPress << QEvent::KeyRelease);
                            
    setBlockedEventTypes(configurePushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                            << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                            << QEvent::KeyPress << QEvent::KeyRelease);
 
    widgetList << enabledCheckBox << configurePushButton;
 
    return widgetList;
}
 
void FcitxAddonSelector::Private::AddonDelegate::updateItemWidgets(const QList<QWidget*> widgets,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QPersistentModelIndex &index) const
{
    QCheckBox *checkBox = static_cast<QCheckBox*>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(addonSelector_d->dependantLayoutValue(MARGIN, checkBox->sizeHint().width(), option.rect.width()), option.rect.height() / 2 - checkBox->sizeHint().height() / 2);
  
    KPushButton *configurePushButton = static_cast<KPushButton*>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(addonSelector_d->dependantLayoutValue(option.rect.width() - MARGIN - configurePushButtonSizeHint.width(), configurePushButtonSizeHint.width(), option.rect.width()), option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);
 
    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        configurePushButton->setVisible(false);
    } else {
        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setEnabled(index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setVisible(index.model()->data(index, ConfigurableRole).toBool());
    }
}

void FcitxAddonSelector::Private::AddonDelegate::slotStateChanged(bool changed)
{
    Q_UNUSED(changed)
    if (!focusedIndex().isValid())
        return;
}

void FcitxAddonSelector::Private::AddonDelegate::emitChanged()
{
    emit changed(true);
}

void FcitxAddonSelector::Private::AddonDelegate::slotConfigureClicked()
{
    KDialog configDialog;
    const QModelIndex index = focusedIndex();
    
    FcitxAddon* addonEntry = static_cast<FcitxAddon*>(index.internalPointer());
    ConfigFileDesc* cfdesc = this->addonSelector_d->parent->parent->configDescManager()->GetConfigDesc(QString::fromUtf8(addonEntry->name).append(".desc"));
    
    if (cfdesc)
    {
        FcitxConfigPage* configPage = new FcitxConfigPage(&configDialog, cfdesc, QString::fromUtf8("conf"), QString::fromUtf8(addonEntry->name).append(".config"));
        configDialog.setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Default);
        configDialog.setMainWidget(configPage);
        connect(&configDialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), configPage, SLOT(buttonClicked(KDialog::ButtonCode)));
        if (configDialog.exec() == QDialog::Accepted)
        {
        }
        else
        {
        }
    }
}



void FcitxAddonSelector::load()
{

}

void FcitxAddonSelector::save()
{

}

FcitxAddonSelector::FcitxAddonSelector(Module* parent):
        QWidget(parent),
        d(new Private(this)),
        parent(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);

    d->lineEdit = new KLineEdit(this);
    d->lineEdit->setClearButtonShown(true);
    d->lineEdit->setClickMessage(i18n("Search Addons"));
    d->listView = new KCategorizedView(this);
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);
    d->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->categoryDrawer= new KCategoryDrawerV3(d->listView);
    d->listView->setCategoryDrawer(d->categoryDrawer);

    d->proxyModel = new Private::ProxyModel(d, this);
    d->addonModel = new Private::AddonModel(d, this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->addonModel);
    d->listView->setModel(d->proxyModel);
    d->listView->setAlternatingBlockColors(true);

    Private::AddonDelegate *addonDelegate = new Private::AddonDelegate(d, this);
    d->listView->setItemDelegate(addonDelegate);

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, SIGNAL(textChanged(QString)), d->proxyModel, SLOT(invalidate()));
    connect(addonDelegate, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(addonDelegate, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));

    layout->addWidget(d->lineEdit);
    layout->addWidget(d->listView);
    this->setLayout(layout);
}

void FcitxAddonSelector::addAddon(FcitxAddon* fcitxAddon)
{
    d->addonModel->addAddon(fcitxAddon);
    d->proxyModel->sort(0);
}


QFont FcitxAddonSelector::Private::AddonDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);
 
    return retFont;
}

FcitxAddonSelector::~FcitxAddonSelector()
{
    delete d->listView->itemDelegate();
    delete d->listView; // depends on some other things in d, make sure this dies first.
    delete d;
}


#include "moc_FcitxAddonSelector.cpp"
#include "moc_FcitxAddonSelector_p.cpp"