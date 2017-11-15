/*
* Copyright (C) 2017~2017 by CSSlayer
* wengxt@gmail.com
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of the
* License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; see the file COPYING. If not,
* see <http://www.gnu.org/licenses/>.
*/
#ifndef _KCM_FCITX_ISO639_H_
#define _KCM_FCITX_ISO639_H_

#include <KLocalizedString>
#include <QMap>
#include <QString>

namespace fcitx {
namespace kcm {

class Iso639 {
public:
    Iso639();

    QString query(const QString &code) const {
        auto value = iso639_2data_.value(code);
        if (!value.isEmpty()) {
            return i18nd("iso_639-2", value.toUtf8().constData());
        }
        value = iso639_3data_.value(code);
        if (!value.isEmpty()) {
            return i18nd("iso_639-3", value.toUtf8().constData());
        }
        value = iso639_5data_.value(code);
        if (!value.isEmpty()) {
            return i18nd("iso_639-5", value.toUtf8().constData());
        }
        return value;
    }

private:
    QMap<QString, QString> iso639_2data_;
    QMap<QString, QString> iso639_3data_;
    QMap<QString, QString> iso639_5data_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_ISO639_H_
