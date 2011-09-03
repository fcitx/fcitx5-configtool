#ifndef FCITXSUBCONFIG_H
#define FCITXSUBCONFIG_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSet>

namespace Fcitx
{
    enum SubConfigType
    {
        SC_None,
        SC_ConfigFile,
        SC_NativeFile
    };

    class FcitxSubConfig: public QObject
    {
        Q_OBJECT
    public:
        static FcitxSubConfig* GetConfigFileSubConfig(const QString& name, const QString& configdesc, const QSet< QString >& fileList);
        static FcitxSubConfig* GetNativeFileSubConfig(const QString& name, const QString& nativepath, const QSet< QString >& fileList);
        SubConfigType type();
        QSet< QString >& filelist();
        const QString& name() const;
        const QString& configdesc() const;
        const QString& nativepath() const;
    private:
        FcitxSubConfig ( QObject* parent = 0 );
        SubConfigType m_type;
        QString m_name;
        QSet< QString > m_filelist;
        QString m_configdesc;
        QString m_nativepath;
    };

}

#endif