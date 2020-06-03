/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _WIDGETSADDONS_QTKEYTRANS_H_
#define _WIDGETSADDONS_QTKEYTRANS_H_

#include <qnamespace.h>

namespace fcitx {
namespace kcm {

bool qEventToSym(int key, Qt::KeyboardModifiers mod, const QString &text,
                 int &outsym, unsigned int &outstate);
}
} // namespace fcitx

#endif // _WIDGETSADDONS_QTKEYTRANS_H_
