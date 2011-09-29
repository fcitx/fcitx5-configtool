#ifndef FCITX_IM_H
#define FCITX_IM_H

#include <QString>
#include <QMetaType>
#include <QDebug>
#include <QDBusArgument>

class FcitxIM {
    public:
        const QString& name() const;
        const QString& uniqueName() const;
        const QString& langCode() const;
        bool enabled() const;
        
        void setName(const QString& name);
        void setUniqueName(const QString& name);
        void setLangCode(const QString& name);
        void setEnabled(bool name);
        static void registerMetaType();
        
        bool operator < (const FcitxIM& im) const
        {
            if (m_enabled == true && im.m_enabled == false)
                return true;
            return false;
        }
    private:
        QString m_name;
        QString m_uniqueName;
        QString m_langCode;
        bool m_enabled;
};

typedef QList<FcitxIM> FcitxIMList;

inline QDebug &operator<<(QDebug& debug, const FcitxIM& im) 
{
    debug << im.name() << " " << im.uniqueName() << " " << im.langCode() << " " << im.enabled();
    return debug;
}

QDBusArgument& operator<<(QDBusArgument& argument, const FcitxIM& im);
const QDBusArgument& operator>>(const QDBusArgument& argument, FcitxIM& im);

Q_DECLARE_METATYPE(FcitxIM)
Q_DECLARE_METATYPE(FcitxIMList)

#endif