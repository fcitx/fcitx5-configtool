#include "FcitxSubConfigPath.h"

namespace Fcitx
{
    FcitxSubConfigPath::FcitxSubConfigPath ( const QString& prefix, const QString& path )
        : m_prefix(prefix), m_path(path)
    {
    }

    const QString& FcitxSubConfigPath::path() const
    {
        return m_path;
    }

    const QString& FcitxSubConfigPath::prefix() const
    {
        return m_prefix;
    }

}