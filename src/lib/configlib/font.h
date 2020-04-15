//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _CONFIGLIB_FONT_H_
#define _CONFIGLIB_FONT_H_

#include <QFont>
#include <QString>

namespace fcitx {
namespace kcm {

QFont parseFont(const QString &str);
QString fontToString(const QFont &font);

} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_FONT_H_
