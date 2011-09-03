#ifndef FCITXSUBCONFIGWIDGET_P_H
#define FCITXSUBCONFIGWIDGET_P_H
#include <QAbstractListModel>

namespace Fcitx
{
    class FcitxConfigFile
    {
    public:
        FcitxConfigFile(const QString& path);
        QString name();
        const QString& path() const;
    private:
        QString m_path;
    };

    class FcitxConfigFileItemModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        FcitxConfigFileItemModel(QObject* parent = 0);
        virtual ~FcitxConfigFileItemModel();
        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        void addConfigFile(FcitxConfigFile* configfile);
    private:
        QList<FcitxConfigFile*> m_files;
    };
}

#endif
