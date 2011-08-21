#include "FcitxSubConfig.h"

namespace Fcitx
{

    FcitxSubConfig* FcitxSubConfig::GetConfigFileSubConfig( const QString& name, const QString& configdesc, const QMultiMap< QString, Fcitx::FcitxSubConfigPath >& fileList)
    {
        FcitxSubConfig* subconfig = new FcitxSubConfig;
        subconfig->m_name = name;
        subconfig->m_type = SC_ConfigFile;
        subconfig->m_filelist = fileList;
        subconfig->m_configdesc = configdesc;
        return subconfig;
    }

    FcitxSubConfig* FcitxSubConfig::GetNativeFileSubConfig( const QString& name, const QString& nativepath, const QMultiMap< QString, Fcitx::FcitxSubConfigPath >& fileList)
    {
        FcitxSubConfig* subconfig = new FcitxSubConfig;
        subconfig->m_name = name;
        subconfig->m_type = SC_NativeFile;
        subconfig->m_filelist = fileList;
        subconfig->m_nativepath = nativepath;
        return subconfig;
    }

    FcitxSubConfig::FcitxSubConfig ( QObject* parent ) : QObject ( parent )
    {

    }

    SubConfigType FcitxSubConfig::type()
    {
        return m_type;
    }

    const QString& FcitxSubConfig::name() const
    {
        return m_name;
    }

    const QString& FcitxSubConfig::configdesc() const
    {
        return m_configdesc;
    }

    const QString& FcitxSubConfig::nativepath() const
    {
        return m_nativepath;
    }

    QMultiMap< QString, FcitxSubConfigPath >& FcitxSubConfig::filelist()
    {
        return m_filelist;
    }

}