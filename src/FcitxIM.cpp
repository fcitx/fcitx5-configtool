#include "FcitxIM.h"
#include <qdbusargument.h>
#include <qdbusmetatype.h>

bool FcitxIM::enabled() const
{
    return m_enabled;
}
const QString& FcitxIM::langCode() const
{
    return m_langCode;
}
const QString& FcitxIM::name() const
{
    return m_name;
}
const QString& FcitxIM::uniqueName() const
{
    return m_uniqueName;
}
void FcitxIM::setEnabled(bool enable)
{
    m_enabled = enable;
}
void FcitxIM::setLangCode(const QString& lang)
{
    m_langCode = lang;
}
void FcitxIM::setName(const QString& name)
{
    m_name = name;
}
void FcitxIM::setUniqueName(const QString& name)
{
    m_uniqueName = name;
}

void FcitxIM::registerMetaType()
{
    qRegisterMetaType<FcitxIM>("FcitxIM"); 
    qDBusRegisterMetaType<FcitxIM>();
    qRegisterMetaType<FcitxIMList>("FcitxIMList"); 
    qDBusRegisterMetaType<FcitxIMList>();
}

QDBusArgument& operator<<(QDBusArgument& argument, const FcitxIM& im)
{
    argument.beginStructure();
    argument << im.name();
    argument << im.uniqueName();
    argument << im.langCode();
    argument << im.enabled();
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, FcitxIM& im)
{
    QString name;
    QString uniqueName;
    QString langCode;
    bool enabled;
    argument.beginStructure();
    argument >> name >> uniqueName >> langCode >> enabled;
    argument.endStructure();
    im.setName(name);
    im.setUniqueName(uniqueName);
    im.setLangCode(langCode);
    im.setEnabled(enabled);
    return argument;
}